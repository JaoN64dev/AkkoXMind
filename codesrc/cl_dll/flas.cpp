
#include "extdll.h"
#include "cl_dll.h"
#include "cbase.h"
#include "player.h"
#include "r_efx.h"
#include "event_api.h"
#include "mathlib.h"




void EV_Teleport ( event_args_t *args )
{
    vec3_t vecOrg;

    VectorCopy( args->origin, vecOrg );

    switch (gEngfuncs.pfnRandomLong(0, 4))
    {
       case 0 : gEngfuncs.pEventAPI->EV_PlaySound( 0, vecOrg, CHAN_STATIC, "misc/r_tele1.wav", 1.0, ATTN_NORM, 0, PITCH_NORM ); break;
       case 1 : gEngfuncs.pEventAPI->EV_PlaySound( 0, vecOrg, CHAN_STATIC, "misc/r_tele2.wav", 1.0, ATTN_NORM, 0, PITCH_NORM ); break;
       case 2 : gEngfuncs.pEventAPI->EV_PlaySound( 0, vecOrg, CHAN_STATIC, "misc/r_tele3.wav", 1.0, ATTN_NORM, 0, PITCH_NORM ); break;
       case 3 : gEngfuncs.pEventAPI->EV_PlaySound( 0, vecOrg, CHAN_STATIC, "misc/r_tele4.wav", 1.0, ATTN_NORM, 0, PITCH_NORM ); break;
       case 4 : gEngfuncs.pEventAPI->EV_PlaySound( 0, vecOrg, CHAN_STATIC, "misc/r_tele5.wav", 1.0, ATTN_NORM, 0, PITCH_NORM ); break;
    }

    gEngfuncs.pEfxAPI->R_TeleportSplash( vecOrg );
}
