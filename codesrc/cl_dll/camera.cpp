// camera.cpp - simple camera weapon (server-side)

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"

#include "cameraweapon.h"

// message id (registered during Precache)
int gmsgScreenFade = 0;

// animation enum (kept minimal)
enum camera_e {
	CAMERA_IDLE1 = 0,
	CAMERA_IDLE2,
	CAMERA_IDLE3,
	CAMERA_SNAP,
	CAMERA_DRAW,
	CAMERA_HOLSTER
};

LINK_ENTITY_TO_CLASS(weapon_camera, CCamera);

// ----------------------------- Spawn -----------------------------
void CCamera::Spawn(void)
{
	Precache();

	// Give a weapon id — use WEAPON_NONE if you prefer a custom system.
	m_iId = WEAPON_NONE;

	SET_MODEL(ENT(pev), "models/w_camera.mdl");

	m_fFlashOn = 0; // flash off by default

	FallInit(); // allow to fall to the ground
}

// ----------------------------- Precache ---------------------------
void CCamera::Precache(void)
{
	PRECACHE_MODEL("models/v_camera.mdl");
	PRECACHE_MODEL("models/w_camera.mdl");
	PRECACHE_MODEL("models/p_camera.mdl");

	PRECACHE_SOUND("camera/click.wav");
	PRECACHE_SOUND("camera/flash.wav");

	// Register the built-in "ScreenFade" user message so we can trigger an instant white flash clientside.
	// If your client HUD registers ScreenFade differently, adjust accordingly.
	gmsgScreenFade = REG_USER_MSG("ScreenFade", -1);
}

// ----------------------------- ItemInfo --------------------------
int CCamera::GetItemInfo(ItemInfo* p)
{
	memset(p, 0, sizeof(*p));
	p->pszName = "weapon_camera";
	p->pszAmmo1 = NULL;
	p->iMaxAmmo1 = -1;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;

	p->iMaxClip = -1; // no clip
	p->iSlot = 1;
	p->iPosition = 1;
	p->iFlags = 0;
	p->iId = m_iId;
	p->iWeight = 0;

	return 1;
}

// ----------------------------- Deploy ----------------------------
BOOL CCamera::Deploy(void)
{
	return DefaultDeploy("models/v_camera.mdl", "models/p_camera.mdl", CAMERA_DRAW, "onehanded");
}

// ----------------------------- Holster ---------------------------
void CCamera::Holster(int skiplocal)
{
	SendWeaponAnim(CAMERA_HOLSTER, 1);
}

// ----------------------------- PrimaryAttack ---------------------
void CCamera::PrimaryAttack(void)
{
	CameraSnap(0.8f);
}

// ----------------------------- CameraSnap -----------------------
void CCamera::CameraSnap(float flCycleTime)
{
	// player attack animation + viewmodel anim
	if (m_pPlayer)
		m_pPlayer->SetAnimation(PLAYER_ATTACK1);

	SendWeaponAnim(CAMERA_SNAP, 1);

	// play sound for click or flash
	if (m_fFlashOn)
		EMIT_SOUND_DYN(edict(), CHAN_WEAPON, "camera/flash.wav", 1.0f, ATTN_NORM, 0, 100);
	else
		EMIT_SOUND_DYN(edict(), CHAN_WEAPON, "camera/click.wav", 1.0f, ATTN_NORM, 0, 100);

	// Instant full-white flash (no fade-in): send ScreenFade user message to the client.
	if (m_fFlashOn)
	{
		m_pPlayer->pev->effects |= EF_BRIGHTLIGHT;

		// remove the light shortly after
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.1f;
	}

	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + flCycleTime;
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 3.0f;
}

// ----------------------------- SecondaryAttack -------------------
void CCamera::SecondaryAttack(void)
{
	ToggleFlash();
	m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.3f;
}

// ----------------------------- ToggleFlash ----------------------
void CCamera::ToggleFlash(void)
{
	m_fFlashOn = !m_fFlashOn;

	// Notify only the player who toggled it.
	if (m_pPlayer)
	{
		ClientPrint(m_pPlayer->pev, HUD_PRINTCENTER, m_fFlashOn ? "Flash: ON" : "Flash: OFF");
	}
}

// ----------------------------- WeaponIdle -----------------------
void CCamera::WeaponIdle(void)
{
	ResetEmptySound();

	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;

	if (!m_pPlayer)
		return;

	float flRand = UTIL_SharedRandomFloat(m_pPlayer->random_seed, 0.0f, 1.0f);
	int iAnim;

	if (flRand <= 0.33f)
	{
		iAnim = CAMERA_IDLE1;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 60.0 / 16.0;
	}
	else if (flRand <= 0.66f)
	{
		iAnim = CAMERA_IDLE2;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 40.0 / 16.0;
	}
	else
	{
		iAnim = CAMERA_IDLE3;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 49.0 / 16.0;
	}

	SendWeaponAnim(iAnim, 1);
}
