#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
class CComputer : public CBaseEntity
{
public:
	void Spawn() override;
	int ObjectCaps() override
	{
		return FCAP_IMPULSE_USE;
	}
	void Think() override;
	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value) override;
};

void CComputer::Spawn()
{   
	SetMovedir(pev);
	pev->movetype = MOVETYPE_PUSH;
	pev->solid = SOLID_BSP;
	SET_MODEL(ENT(pev), STRING(pev->model));
	SetTouch(NULL);
	SetUse(&CComputer::Use);
}

void CComputer::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value){

}
