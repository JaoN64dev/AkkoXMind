#ifndef WEAPON_CAMERA_H
#define WEAPON_CAMERA_H

#include "weapons.h"

class CCamera : public CBasePlayerWeapon
{
public:
    void Spawn(void);
    void Precache(void);
    int iItemSlot(void) { return 2; }
    int GetItemInfo(ItemInfo* p);

    bool Deploy(void);
    void PrimaryAttack(void);
    void SecondaryAttack(void);
    void SnapPic(float flSpread, float flCycleTime, bool fUseAutoAim);

    void Reload(void);
    void WeaponIdle(void);

    virtual bool UseDecrement(void)
    {
#if defined( CLIENT_WEAPONS )
        return true;
#else
        return false;
#endif
    }

    int m_iFlashMode;
};

#endif