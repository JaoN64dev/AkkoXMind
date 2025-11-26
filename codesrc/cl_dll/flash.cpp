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


#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"
#include "r_efx.h"
#include <string.h>
#include <stdio.h>
#include "dlight.h"
#include "event_api.h"

DECLARE_MESSAGE(m_Flashy, Flash)

int CHudFlash::Init(void)
{
	HOOK_MESSAGE(Flashy);
}


void CHudFlash::MsgFunc_Flash(const char *pszName, int iSize, void *pbuf) {

		vec3_t vecOrg;
		VectorCopy(args->origin, vecOrg);

		switch (gEngfuncs.pfnRandomLong(0, 4))
		{
		case 0: gEngfuncs.pEventAPI->EV_PlaySound(0, vecOrg, CHAN_STATIC, "misc/r_tele1.wav", 1.0, ATTN_NORM, 0, PITCH_NORM); break;
		case 1: gEngfuncs.pEventAPI->EV_PlaySound(0, vecOrg, CHAN_STATIC, "misc/r_tele2.wav", 1.0, ATTN_NORM, 0, PITCH_NORM); break;
		case 2: gEngfuncs.pEventAPI->EV_PlaySound(0, vecOrg, CHAN_STATIC, "misc/r_tele3.wav", 1.0, ATTN_NORM, 0, PITCH_NORM); break;
		case 3: gEngfuncs.pEventAPI->EV_PlaySound(0, vecOrg, CHAN_STATIC, "misc/r_tele4.wav", 1.0, ATTN_NORM, 0, PITCH_NORM); break;
		case 4: gEngfuncs.pEventAPI->EV_PlaySound(0, vecOrg, CHAN_STATIC, "misc/r_tele5.wav", 1.0, ATTN_NORM, 0, PITCH_NORM); break;
		}

		gEngfuncs.pEfxAPI->R_TeleportSplash(vecOrg);
	
}