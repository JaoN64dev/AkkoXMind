#pragma once
#ifndef CAMERA_H
#define CAMERA_H

// HL SDK includes
#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"

// Forward: message id defined in camera.cpp
extern int gmsgScreenFade;

class CCamera : public CBasePlayerWeapon
{
public:
	int m_fFlashOn; // 0 = off, 1 = on

public:
	void Spawn(void);
	void Precache(void);
	int GetItemInfo(ItemInfo* p);

	BOOL Deploy(void);
	void Holster(int skiplocal);

	void PrimaryAttack(void);   // take picture
	void SecondaryAttack(void); // toggle flash

	void CameraSnap(float flCycleTime);
	void ToggleFlash(void);

	void WeaponIdle(void);
};

#endif // CAMERA_H
