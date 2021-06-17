// ========= Copyright Open Fortress Developers, CC-BY-NC-SA ============
// Purpose: Implementation of CTFPowerup, base for health kits and ammo kits
// Author(s): Cherry!
//

#include "cbase.h"
#include "of_shareddefs.h"
#include "of_powerup.h"
#include "filesystem.h"
#include "of_gamerules.h"

BEGIN_DATADESC(COFPowerup)
	DEFINE_KEYFIELD(m_bDisabled,FIELD_BOOLEAN,"StartDisabled"),
	DEFINE_KEYFIELD(m_iszModel, FIELD_CHARACTER, "powerup_model"),
	DEFINE_KEYFIELD(m_bAutoMaterialize, FIELD_BOOLEAN, "AutoMaterialize"),

	DEFINE_INPUTFUNC(FIELD_VOID, "Enable", InputEnable),
	DEFINE_INPUTFUNC(FIELD_VOID, "Disable", InputDisable),
	DEFINE_INPUTFUNC(FIELD_VOID, "Toggle", InputToggle),
END_DATADESC();

COFPowerup::COFPowerup()
{
	m_bDisabled = false;
	m_bRespawning = false;
	m_bAutoMaterialize = true;
	m_iszModel = NULL_STRING;
	m_fThrowWaitTime = -1; //0xbf800000

	UseClientSideAnimation();
}

void COFPowerup::Spawn()
{
	Precache();
	SetModel(GetPowerupModel());
	BaseClass::Spawn();

	SetOriginalSpawnOrigin(GetAbsOrigin());
	SetOriginalSpawnAngles(GetAbsAngles());

	VPhysicsDestroyObject();
	SetMoveType(MOVETYPE_NONE);
	SetSolidFlags(FSOLID_TRIGGER | FSOLID_NOT_SOLID);

	//if (m_bDisabled)
	//{
	//	m_bDisabled = true;
	//	AddEffects(EF_NODRAW);
	//}
	SetDisabled(true);

	// (field_0x4ec + 1) = 0;
	m_bRespawning = false;

	ResetSequence(LookupSequence("idle"));
}

void COFPowerup::Precache()
{
	PrecacheModel(GetPowerupModel());
	BaseClass::Precache();
}

void COFPowerup::SetDisabled(bool bDisable)
{
	m_bDisabled = bDisable;

	if (bDisable)
	{
		AddEffects(EF_NODRAW);
	}
	else
	{
		if (!m_bRespawning)
		{
			RemoveEffects(EF_NODRAW);
		}
		else if (!m_bAutoMaterialize)
		{
			Materialize_Internal();
		}
	}
}

void COFPowerup::Materialize()
{
	if (m_bAutoMaterialize)
	{
		Materialize_Internal();
	}
}

void COFPowerup::Materialize_Internal()
{
	if (!m_bDisabled && IsEffectActive(EF_NODRAW))
	{
		EmitSound("Item.Materialize");
		RemoveEffects(EF_NODRAW);
	}

	m_bRespawning = false;

	SetTouch(&CItem::ItemTouch);
}

CBaseEntity *COFPowerup::Respawn()
{
	m_bRespawning = true;

	SetNextThink(gpGlobals->curtime + GetRespawnDelay());

	return BaseClass::Respawn();
}

bool COFPowerup::ValidTouch(CBasePlayer *pPlayer)
{
	if (!pPlayer || !pPlayer->IsPlayer() || !pPlayer->IsAlive()) return false;

	if (m_bDisabled) return false;

	if (GetTeamNumber() != TEAM_UNASSIGNED && GetTeamNumber() != pPlayer->GetTeamNumber()) return false;

	// mvm check, ignore

	return true;
}

bool COFPowerup::MyTouch(CBasePlayer *pPlayer)
{
	return false;
}

void COFPowerup::DropSingleInstance(Vector &pVelocity, CBaseCombatCharacter *pOwner, float param_3, float param_4)
{
	SetMoveType(MOVETYPE_FLYGRAVITY, MOVECOLLIDE_FLY_BOUNCE);
	SetAbsVelocity(pVelocity);
	SetSolid(SOLID_BBOX);

	if (param_4) ActivateWhenAtRest(param_4);

	field_0x4ee = true; // m_bThrown?

	AddSpawnFlags(SF_NORESPAWN); // 0x40000000 = (1 << 30) = 1073741824
	SetOwnerEntity(pOwner);
	m_fThrowWaitTime = gpGlobals->curtime + param_3;

	SetContextThink(&BaseClass::SUB_Remove,gpGlobals->curtime + GetLifeTime(),"PowerupRemoveThink");
}

bool COFPowerup::ItemCanBeTouchedByPlayer(CBasePlayer *pPlayer)
{
	if (pPlayer == GetOwnerEntity() && gpGlobals->curtime < m_fThrowWaitTime)
	{
		return false;
	}

	return BaseClass::ItemCanBeTouchedByPlayer(pPlayer);
}

float COFPowerup::GetRespawnDelay()
{
	return OFGameRules()->FlItemRespawnTime();
}

const char *COFPowerup::GetPowerupModel()
{
	if (m_iszModel != NULL_STRING)
	{
		if (g_pFullFileSystem->FileExists(STRING(m_iszModel), "GAME"))
		{
			return STRING(m_iszModel);
		}
	}

	/* WARNING: Could not recover jumptable at 0x009fdf3a. Too many branches */
	/* WARNING: Treating indirect jump as call */
	// i have a bad feeling this could lead to some huge switch containing what the model should be used
	// so instead ill just do this
	return GetDefaultPowerupModel();
}