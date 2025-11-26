/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
*
*	This product contains software technology licensed from Id
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc.
*	All Rights Reserved.
*
*   Use, distribution, and modification of this source code and/or resulting
*   object code is restricted to non-commercial enhancements to products from
*   Valve LLC.  All other use, distribution, or modification is prohibited
*   without written permission from Valve LLC.
*
****/

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "weapons.h"
#include "nodes.h"
#include "player.h"
#include "soundent.h"
#include "gamerules.h"
#include "r_efx.h"


enum mp5_e
{
	MP5_LONGIDLE = 0,
	MP5_IDLE1,
	MP5_LAUNCH,
	MP5_RELOAD,
	MP5_DEPLOY,
	MP5_FIRE1,
	MP5_FIRE2,
	MP5_FIRE3,
};

const char* cameraTargets[] = {
	"monster_islave",
	"monster_human_grunt",
};

LINK_ENTITY_TO_CLASS(weapon_camera, CCamera);
//=========================================================
//=========================================================


void CCamera::Spawn()
{
	pev->classname = MAKE_STRING("weapon_9mmAR"); // hack to allow for old names
	Precache();
	SET_MODEL(ENT(pev), "models/w_9mmAR.mdl");
	m_iId = WEAPON_MP5;

	m_iDefaultAmmo = MP5_DEFAULT_GIVE;
	m_IdealZoom = 90;
	m_ZoomSpeed = 2.5;
	m_fInZoom = 0;

	FallInit();// get ready to fall down.
}


void CCamera::Precache(void)
{
	PRECACHE_MODEL("models/v_9mmAR.mdl");
	PRECACHE_MODEL("models/w_9mmAR.mdl");
	PRECACHE_MODEL("models/p_9mmAR.mdl");

	PRECACHE_MODEL("models/grenade.mdl");	// grenade

	PRECACHE_MODEL("models/w_9mmARclip.mdl");
	PRECACHE_SOUND("items/9mmclip1.wav");

	PRECACHE_SOUND("items/clipinsert1.wav");
	PRECACHE_SOUND("items/cliprelease1.wav");

	PRECACHE_SOUND("weapons/hks1.wav");// H to the K
	PRECACHE_SOUND("weapons/hks2.wav");// H to the K
	PRECACHE_SOUND("weapons/hks3.wav");// H to the K

	PRECACHE_SOUND("weapons/glauncher.wav");
	PRECACHE_SOUND("weapons/glauncher2.wav");

	PRECACHE_SOUND("weapons/357_cock1.wav");

	m_usMP5 = PRECACHE_EVENT(1, "events/mp5.sc");
	m_usMP52 = PRECACHE_EVENT(1, "events/mp52.sc");
}

int CCamera::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "9mm";
	p->iMaxAmmo1 = _9MM_MAX_CARRY;
	p->pszAmmo2 = "ARgrenades";
	p->iMaxAmmo2 = M203_GRENADE_MAX_CARRY;
	p->iMaxClip = MP5_MAX_CLIP;
	p->iSlot = 1;
	p->iPosition = 0;
	p->iFlags = 0;
	p->iId = m_iId = WEAPON_MP5;
	p->iWeight = MP5_WEIGHT;

	return 1;
}

int CCamera::AddToPlayer(CBasePlayer *pPlayer)
{
	if (CBasePlayerWeapon::AddToPlayer(pPlayer))
	{
		MESSAGE_BEGIN(MSG_ONE, gmsgWeapPickup, NULL, pPlayer->pev);
		WRITE_BYTE(m_iId);
		MESSAGE_END();
		return TRUE;
	}
	return FALSE;
}

BOOL CCamera::Deploy()
{
	BOOL result = DefaultDeploy("models/v_9mmAR.mdl", "models/p_9mmAR.mdl", MP5_DEPLOY, "mp5");

	CLIENT_COMMAND(m_pPlayer->edict(), "cx_vhsborders 1\n");

	return result;
}

void CCamera::Holster(int skiplocal)
{
	CLIENT_COMMAND(m_pPlayer->edict(), "cx_vhsborders 0\n");
	CBasePlayerWeapon::Holster(skiplocal);
}

void CCamera::ItemPostFrame()
{
	// chama o original
	CBasePlayerWeapon::ItemPostFrame();

	float currentzoom = m_pPlayer->pev->fov;

	if (currentzoom == 0)
		currentzoom = 90; // hl usa 0 como "default fov"

	if (currentzoom != m_IdealZoom)
	{
		float delta = m_IdealZoom - currentzoom;

		// velocidade (quanto ele se move por frame)
		float step = m_ZoomSpeed * gpGlobals->frametime;

		// aproxima suavemente
		if (fabs(delta) <= step)
			currentzoom = m_IdealZoom;
		else
			currentzoom += (delta > 0 ? step : -step);

		// aplica
		m_pPlayer->pev->fov = m_pPlayer->m_iFOV = currentzoom;
	}

	m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.0;
}

void CCamera::PrimaryAttack()
{
	// don't fire underwater
	if (m_pPlayer->pev->waterlevel == 3)
	{
		PlayEmptySound();
		m_flNextPrimaryAttack = 0.15;
		return;
	}
	CBaseEntity *pCameratarget= NULL;
	while ((pCameratarget = UTIL_FindEntityInSphere(pCameratarget, pev->origin, 256)) != NULL) {
		const char* entName = STRING(pCameratarget->pev->targetname);
		if (!entName) continue;

		for (const char* target : cameraTargets) {
			if (strcmp(entName, target) == 0) {
			
			}
		}
	}


	



	m_pPlayer->SetAnimation(PLAYER_ATTACK1);

	Vector vecSrc = m_pPlayer->GetGunPosition();
	Vector vecAiming = m_pPlayer->GetAutoaimVector(AUTOAIM_5DEGREES);
	Vector vecDir;

#ifdef CLIENT_DLL
	if (!bIsMultiplayer())
#else
	if (!g_pGameRules->IsMultiplayer())
#endif
	{
	
		vecDir = m_pPlayer->FireBulletsPlayer(1, vecSrc, vecAiming, VECTOR_CONE_1DEGREES, 8192, BULLET_PLAYER_MP5, 2, 0, m_pPlayer->pev, m_pPlayer->random_seed);
	}
	else
	{
		// single player spread
		vecDir = m_pPlayer->FireBulletsPlayer(1, vecSrc, vecAiming, VECTOR_CONE_1DEGREES, 8192, BULLET_PLAYER_MP5, 2, 0, m_pPlayer->pev, m_pPlayer->random_seed);
	}

	int flags;
#if defined( CLIENT_WEAPONS )
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif

	PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usMP5, 0.0, (float *)&g_vecZero, (float *)&g_vecZero, vecDir.x, vecDir.y, 0, 0, 0, 0);

	if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		// HEV suit - indicate out of ammo condition
		m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);

	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.1;

	if (m_flNextPrimaryAttack < UTIL_WeaponTimeBase())
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.1;

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat(m_pPlayer->random_seed, 10, 15);

	if (FlashOn)
	{
		MESSAGE_BEGIN(MSG_ONE, gmsgCameraFlash, NULL, this->pev);
		WRITE_COORD(pev->origin.x);
		WRITE_COORD(pev->origin.y);
		WRITE_COORD(pev->origin.z);
		MESSAGE_END();
	}
	else
	{
		return;  
	}

}

int m_fInZoom;

void CCamera::SecondaryAttack(void)
{
	/*
	// don't fire underwater
	if (m_pPlayer->pev->waterlevel == 3)
	{
	PlayEmptySound();
	m_flNextPrimaryAttack = 0.15;
	return;
	}

	if (m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] == 0)
	{
	PlayEmptySound();
	return;
	}

	m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
	m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;

	m_pPlayer->m_iExtraSoundTypes = bits_SOUND_DANGER;
	m_pPlayer->m_flStopExtraSoundTime = UTIL_WeaponTimeBase() + 0.2;

	m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType]--;

	// player "shoot" animation
	m_pPlayer->SetAnimation(PLAYER_ATTACK1);

	UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);

	// we don't add in player velocity anymore.
	CGrenade::ShootContact(m_pPlayer->pev,
	m_pPlayer->pev->origin + m_pPlayer->pev->view_ofs + gpGlobals->v_forward * 16,
	gpGlobals->v_forward * 800);

	int flags;
	#if defined( CLIENT_WEAPONS )
	flags = FEV_NOTHOST;
	#else
	flags = 0;
	#endif

	PLAYBACK_EVENT(flags, m_pPlayer->edict(), m_usMP52);

	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 1;
	m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1;
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 5;// idle pretty soon after shooting.

	if (!m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType])
	// HEV suit - indicate out of ammo condition
	m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
	*/

	if (m_flNextSecondaryAttack > UTIL_WeaponTimeBase())
		return;

	if (m_fInZoom)
	{
		m_fInZoom = 0;
		m_IdealZoom = 90;
		m_ZoomSpeed = 2.5;
	}
	else
	{
		m_fInZoom = 1;
		m_IdealZoom = 20;
		m_ZoomSpeed = 2.5;
	}

}

void CCamera::Reload()
{
	if (m_flNextReload > gpGlobals->time)
		return;

	m_flNextReload = gpGlobals->time + 0.5f;

	EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", 1, ATTN_NORM);
	if (FlashOn == false) 
	{ 
		FlashOn = true; 
		ALERT(at_console, "On"); 
	}
	else { 
		ALERT(at_console, "Off"); 
		FlashOn = false;
	}
}




void CCamera::WeaponIdle(void)
{
	ResetEmptySound();

	m_pPlayer->GetAutoaimVector(AUTOAIM_5DEGREES);

	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;

	int iAnim;
	switch (RANDOM_LONG(0, 1))
	{
	case 0:
		iAnim = MP5_LONGIDLE;
		break;

	default:
	case 1:
		iAnim = MP5_IDLE1;
		break;
	}

	SendWeaponAnim(iAnim);

	m_flTimeWeaponIdle = UTIL_SharedRandomFloat(m_pPlayer->random_seed, 10, 15); // how long till we do this again.
}



















