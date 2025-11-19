#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapon_camera.h"
#include "dlight.h"

enum camera_e
{
    CAMERA_IDLE1 = 0,
    CAMERA_IDLE2,
    CAMERA_IDLE3,
    CAMERA_SHOOT,
    CAMERA_SHOOT_EMPTY,
    CAMERA_RELOAD,
    CAMERA_RELOAD_NOT_EMPTY,
    CAMERA_DRAW,
    CAMERA_HOLSTER
};

LINK_ENTITY_TO_CLASS(weapon_camera, CCamera);

void CCamera::Spawn()
{
    pev->classname = MAKE_STRING("weapon_camera");
    Precache();
    m_iId = WEAPON_CAMERA;
    SET_MODEL(ENT(pev), "models/w_9mmhandgun.mdl");

    m_iDefaultAmmo = 0;
    m_iFlashMode = 0;

    FallInit();
}

void CCamera::Precache(void)
{
    PRECACHE_MODEL("models/v_9mmhandgun.mdl");
    PRECACHE_MODEL("models/w_9mmhandgun.mdl");
    PRECACHE_MODEL("models/p_9mmhandgun.mdl");

    PRECACHE_SOUND("items/9mmclip1.wav");
    PRECACHE_SOUND("items/9mmclip2.wav");
}

int CCamera::GetItemInfo(ItemInfo* p)
{
    p->pszName = STRING(pev->classname);
    p->pszAmmo1 = NULL;
    p->iMaxAmmo1 = -1;
    p->pszAmmo2 = NULL;
    p->iMaxAmmo2 = -1;
    p->iMaxClip = -1;
    p->iSlot = 1;
    p->iPosition = 0;
    p->iFlags = 0;
    p->iId = m_iId = WEAPON_CAMERA;
    p->iWeight = 0;

    return 1;
}

bool CCamera::Deploy()
{
    return DefaultDeploy("models/v_9mmhandgun.mdl", "models/p_9mmhandgun.mdl", CAMERA_DRAW, "onehanded", 0);
}

void CCamera::SecondaryAttack(void)
{
    m_iFlashMode = !m_iFlashMode;

    if (m_iFlashMode)
        ClientPrint(m_pPlayer->pev, HUD_PRINTCENTER, "Flash: ON\n");
    else
        ClientPrint(m_pPlayer->pev, HUD_PRINTCENTER, "Flash: OFF\n");

    m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.3;
}

void CCamera::PrimaryAttack(void)
{
    SnapPic(0.01, 0.3, TRUE);
}

void CCamera::SnapPic(float flSpread, float flCycleTime, bool fUseAutoAim)
{
    // Flash effect
    if (m_iFlashMode)
    {
        MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, m_pPlayer->pev->origin);
        WRITE_BYTE(TE_DLIGHT);
        WRITE_COORD(m_pPlayer->pev->origin.x);
        WRITE_COORD(m_pPlayer->pev->origin.y);
        WRITE_COORD(m_pPlayer->pev->origin.z + 16);
        WRITE_BYTE(64);
        WRITE_BYTE(255);
        WRITE_BYTE(255);
        WRITE_BYTE(255);
        WRITE_BYTE(1);
        WRITE_BYTE(255);
        MESSAGE_END();
    }

    SendWeaponAnim(CAMERA_SHOOT);

    m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime;
    m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + RANDOM_FLOAT(10, 15);
}

void CCamera::Reload(void)
{
    return;
}

void CCamera::WeaponIdle(void)
{
    ResetEmptySound();
    if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
        return;

    int iAnim;
    float flRand = RANDOM_FLOAT(0, 1);

    if (flRand <= 0.3)
        iAnim = CAMERA_IDLE3;
    else if (flRand <= 0.6)
        iAnim = CAMERA_IDLE1;
    else
        iAnim = CAMERA_IDLE2;

    SendWeaponAnim(iAnim);
    m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 3.0;
}
