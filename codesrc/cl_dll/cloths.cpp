#include "hud.h"
#include "cl_util.h"
#include "triangleapi.h"
#include "r_studioint.h"
#include <vector>
#include <cmath>
#include <string.h>

// --- Vec3 helper struct ---
struct Vec3 {
    float x, y, z;
    Vec3() :x(0), y(0), z(0) {}
    Vec3(float X, float Y, float Z) :x(X), y(Y), z(Z) {}
    Vec3(const vec3_t& v) { x = v[0]; y = v[1]; z = v[2]; }
    operator vec3_t() const { vec3_t out; out[0] = x; out[1] = y; out[2] = z; return out; }
    Vec3 operator+(const Vec3& o) const { return Vec3(x + o.x, y + o.y, z + o.z); }
    Vec3 operator-(const Vec3& o) const { return Vec3(x - o.x, y - o.y, z - o.z); }
    Vec3 operator*(float s) const { return Vec3(x * s, y * s, z * s); }
    Vec3& operator+=(const Vec3& o) { x += o.x; y += o.y; z += o.z; return *this; }
    float length() const { return sqrtf(x * x + y * y + z * z); }
    Vec3 normalized() const {
        float len = length();
        return (len > 0.0001f) ? (*this * (1.0f / len)) : Vec3(0, 0, 0);
    }
};

// --- HL1 Cloth class ---
class ClothHL {
public:
    int cols, rows;
    float spacing;
    std::vector<Vec3> pos;
    std::vector<Vec3> oldPos;
    std::vector<Vec3> accel;
    std::vector<char> pinned;
    std::vector<float> texU, texV;

    // settings
    Vec3 gravity;
    float damping;
    int constraintIters;
    float windMagnitude;
    Vec3 windDirection;

    // WAD texture handle
    int textureId;
    char textureName[64];
    bool textureLoaded;

    double lastTime;

    ClothHL() : cols(0), rows(0), spacing(0.0f), textureId(0), textureLoaded(false) {
        gravity = Vec3(0.0f, -600.0f, 0.0f);
        damping = 0.99f;
        constraintIters = 5;
        windMagnitude = 0.0f;
        windDirection = Vec3(1, 0, 0);
        lastTime = 0.0;
        textureName[0] = '\0';
    }

    inline int idx(int c, int r) const { return r * cols + c; }

    void CreateGrid(int c, int r, float spacing_, const Vec3& origin, const Vec3& right, const Vec3& down) {
        cols = c; rows = r; spacing = spacing_;
        int N = cols * rows;
        pos.assign(N, Vec3());
        oldPos.assign(N, Vec3());
        accel.assign(N, Vec3());
        pinned.assign(N, 0);
        texU.assign(N, 0.0f);
        texV.assign(N, 0.0f);

        for (int rr = 0; rr < rows; ++rr) {
            for (int cc = 0; cc < cols; ++cc) {
                int i = idx(cc, rr);
                pos[i] = origin + right * (cc * spacing) + down * (rr * spacing);
                oldPos[i] = pos[i];
                accel[i] = Vec3(0, 0, 0);
                texU[i] = float(cc) / float(cols - 1);
                texV[i] = float(rr) / float(rows - 1);
            }
        }
        lastTime = gEngfuncs.GetClientTime();
    }

    void PinTopEdge() {
        for (int c = 0; c < cols; ++c)
            pinned[idx(c, 0)] = 1;
    }

    void PinCorners() {
        pinned[idx(0, 0)] = 1;
        pinned[idx(cols - 1, 0)] = 1;
    }

    // Load texture from WAD file
    bool LoadWADTexture(const char* name) {
        if (!name || name[0] == '\0') return false;

        strncpy(textureName, name, sizeof(textureName) - 1);
        textureName[sizeof(textureName) - 1] = '\0';

        // Find texture in WAD
        textureId = gEngfuncs.pTriAPI->FindTexture(textureName);

        if (textureId > 0) {
            textureLoaded = true;
            gEngfuncs.Con_Printf("Cloth: Loaded WAD texture '%s' (ID: %d)\n", textureName, textureId);
            return true;
        }
        else {
            textureLoaded = false;
            gEngfuncs.Con_Printf("Cloth: Failed to load WAD texture '%s'\n", textureName);
            return false;
        }
    }

    void Update(float dt) {
        if (dt <= 0.0f || dt > 0.1f) return; // Safety check

        int N = cols * rows;

        // Apply forces
        for (int i = 0; i < N; ++i) {
            if (pinned[i]) continue;

            // Gravity
            accel[i] = gravity;

            // Wind (simple)
            if (windMagnitude > 0.0f) {
                accel[i] += windDirection * windMagnitude;
            }
        }

        // Verlet integration
        for (int i = 0; i < N; ++i) {
            if (pinned[i]) continue;

            Vec3 vel = pos[i] - oldPos[i];
            Vec3 newPos = pos[i] + vel * damping + accel[i] * (dt * dt);
            oldPos[i] = pos[i];
            pos[i] = newPos;
        }

        // Constraint solving
        for (int iter = 0; iter < constraintIters; ++iter) {
            // Structural constraints (horizontal and vertical)
            for (int r = 0; r < rows; ++r) {
                for (int c = 0; c < cols - 1; ++c) {
                    SolveConstraint(idx(c, r), idx(c + 1, r), spacing);
                }
            }
            for (int c = 0; c < cols; ++c) {
                for (int r = 0; r < rows - 1; ++r) {
                    SolveConstraint(idx(c, r), idx(c, r + 1), spacing);
                }
            }

            // Shear constraints (diagonals)
            for (int r = 0; r < rows - 1; ++r) {
                for (int c = 0; c < cols - 1; ++c) {
                    float diag = spacing * sqrtf(2.0f);
                    SolveConstraint(idx(c, r), idx(c + 1, r + 1), diag);
                    SolveConstraint(idx(c + 1, r), idx(c, r + 1), diag);
                }
            }
        }
    }

    void SolveConstraint(int i1, int i2, float restLen) {
        Vec3 delta = pos[i2] - pos[i1];
        float len = delta.length();
        if (len < 0.0001f) return;

        float diff = (len - restLen) / len;
        Vec3 correction = delta * (diff * 0.5f);

        if (!pinned[i1]) pos[i1] += correction;
        if (!pinned[i2]) pos[i2] -= correction;
    }

    void Render() {
        if (!textureLoaded || cols < 2 || rows < 2) return;

        // Bind WAD texture
        gEngfuncs.pTriAPI->RenderMode(kRenderNormal);
        gEngfuncs.pTriAPI->Bind(textureId, 0);
        gEngfuncs.pTriAPI->CullFace(TRI_NONE); // Render both sides

        // Draw triangles
        gEngfuncs.pTriAPI->Begin(TRI_TRIANGLES);

        for (int r = 0; r < rows - 1; ++r) {
            for (int c = 0; c < cols - 1; ++c) {
                int i0 = idx(c, r);
                int i1 = idx(c + 1, r);
                int i2 = idx(c + 1, r + 1);
                int i3 = idx(c, r + 1);

                // First triangle (i0, i1, i2)
                gEngfuncs.pTriAPI->TexCoord2f(texU[i0], texV[i0]);
                gEngfuncs.pTriAPI->Vertex3f(pos[i0].x, pos[i0].y, pos[i0].z);

                gEngfuncs.pTriAPI->TexCoord2f(texU[i1], texV[i1]);
                gEngfuncs.pTriAPI->Vertex3f(pos[i1].x, pos[i1].y, pos[i1].z);

                gEngfuncs.pTriAPI->TexCoord2f(texU[i2], texV[i2]);
                gEngfuncs.pTriAPI->Vertex3f(pos[i2].x, pos[i2].y, pos[i2].z);

                // Second triangle (i0, i2, i3)
                gEngfuncs.pTriAPI->TexCoord2f(texU[i0], texV[i0]);
                gEngfuncs.pTriAPI->Vertex3f(pos[i0].x, pos[i0].y, pos[i0].z);

                gEngfuncs.pTriAPI->TexCoord2f(texU[i2], texV[i2]);
                gEngfuncs.pTriAPI->Vertex3f(pos[i2].x, pos[i2].y, pos[i2].z);

                gEngfuncs.pTriAPI->TexCoord2f(texU[i3], texV[i3]);
                gEngfuncs.pTriAPI->Vertex3f(pos[i3].x, pos[i3].y, pos[i3].z);
            }
        }

        gEngfuncs.pTriAPI->End();
    }
};

// --- Global cloth instance ---
ClothHL g_Cloth;

// --- Initialization function ---
void InitCloth() {
    // Create cloth grid
    // origin, right direction, down direction
    Vec3 origin(0, 0, 200);     // Position in world
    Vec3 right(1, 0, 0);        // X direction
    Vec3 down(0, 0, -1);        // Down in Z (HL1 uses Z-up)

    g_Cloth.CreateGrid(20, 20, 10.0f, origin, right, down);
    g_Cloth.PinTopEdge();

    // Load texture from WAD - CHANGE THIS to your texture name
    g_Cloth.LoadWADTexture("{blue");  // Example: "{blue" (liquid texture)
    // Other examples: "CRETE1_FLR", "{GRAFITI2", "METAL1_4", etc.

    // Configure physics
    g_Cloth.gravity = Vec3(0, 0, -980);  // HL1 gravity (Z is down)
    g_Cloth.damping = 0.99f;
    g_Cloth.constraintIters = 5;
    g_Cloth.windMagnitude = 50.0f;
    g_Cloth.windDirection = Vec3(1, 0, 0);
}

// --- Update function (call from HUD_UpdateClientData) ---
void UpdateCloth() {
    double currentTime = gEngfuncs.GetClientTime();
    float dt = (float)(currentTime - g_Cloth.lastTime);
    g_Cloth.lastTime = currentTime;

    if (dt > 0.0f && dt < 0.1f) {
        g_Cloth.Update(dt);
    }
}

// --- Export function for rendering ---
extern "C" void DLLEXPORT HUD_DrawTransparentTriangles() {
    gEngfuncs.pTriAPI->RenderMode(kRenderNormal);
    g_Cloth.Render();
}

// --- Console command to change texture at runtime ---
void ClothTexture_f() {
    if (gEngfuncs.Cmd_Argc() < 2) {
        gEngfuncs.Con_Printf("Usage: cloth_texture <texture_name>\n");
        gEngfuncs.Con_Printf("Example: cloth_texture {blue\n");
        return;
    }

    const char* texName = gEngfuncs.Cmd_Argv(1);
    g_Cloth.LoadWADTexture(texName);
}

// --- Register console command (call from HUD_Init) ---
void RegisterClothCommands() {
    gEngfuncs.pfnAddCommand("cloth_texture", ClothTexture_f);
}

// --- Hook these into your HUD code ---
// In HUD_Init(): call InitCloth() and RegisterClothCommands()
// In HUD_UpdateClientData(): call UpdateCloth()
// HUD_DrawTransparentTriangles is already exported above