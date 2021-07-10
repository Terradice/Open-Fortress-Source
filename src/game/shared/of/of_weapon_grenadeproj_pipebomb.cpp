// ========= Copyright Open Fortress Developers, CC-BY-NC-SA ============
// Purpose: Implementation of CTFGrenadePipeBombProjectile
// Author(s): Cherry!
//

#include "cbase.h"
#include "of_shareddefs.h"
#include "of_weapon_base.h"
#include "of_weapon_grenadeproj_pipebomb.h"
#include "IEffects.h"
#ifdef GAME_DLL
#include "props.h"
#include "physics_collisionevent.h"
#endif

// ----------------------------------------------------------------------------- //
// Data tables
// ----------------------------------------------------------------------------- //

IMPLEMENT_NETWORKCLASS_ALIASED(OFGrenadePipeBombProjectile, DT_OFProjectile_PipeBomb)

BEGIN_NETWORK_TABLE(COFGrenadePipeBombProjectile, DT_OFProjectile_PipeBomb)
#ifdef CLIENT_DLL
	RecvPropInt(RECVINFO(m_bTouched)),
	RecvPropInt(RECVINFO(m_iType)),
	RecvPropEHandle(RECVINFO(m_hLauncher)),
#else
	SendPropInt(SENDINFO(m_bTouched)),
	SendPropInt(SENDINFO(m_iType)),
	SendPropEHandle(SENDINFO(m_hLauncher)),
#endif
END_NETWORK_TABLE()

#ifdef GAME_DLL
LINK_ENTITY_TO_CLASS(tf_projectile_pipe, COFGrenadePipeBombProjectile);
PRECACHE_REGISTER(tf_projectile_pipe);
LINK_ENTITY_TO_CLASS(tf_projectile_pipe_remote, COFGrenadePipeBombProjectile);
PRECACHE_REGISTER(tf_projectile_pipe_remote);
#endif

// OFTODO: rename them to tf_pipebomb_!
ConVar tf_grenade_forcefrom_bullet("tf_grenade_forcefrom_bullet", "2.0", FCVAR_NOTIFY | FCVAR_CHEAT | FCVAR_REPLICATED);
ConVar tf_grenade_forcefrom_buckshot("tf_grenade_forcefrom_buckshot", "0.75", FCVAR_NOTIFY | FCVAR_CHEAT | FCVAR_REPLICATED);
ConVar tf_grenade_forcefrom_blast("tf_grenade_forcefrom_blast", "0.15", FCVAR_NOTIFY | FCVAR_CHEAT | FCVAR_REPLICATED);
ConVar tf_pipebomb_force_to_move("tf_pipebomb_force_to_move", "1500.0", FCVAR_NOTIFY | FCVAR_CHEAT | FCVAR_REPLICATED);
ConVar tf_grenade_force_sleeptime("tf_grenade_force_sleeptime", "1.0", FCVAR_NOTIFY | FCVAR_CHEAT | FCVAR_REPLICATED);
ConVar tf_pipebomb_deflect_reset_time("tf_pipebomb_deflect_reset_time", "10.0", FCVAR_NOTIFY | FCVAR_CHEAT | FCVAR_REPLICATED);

// OFTODO: move to grenade launcher!
ConVar tf_grenadelauncher_livetime("tf_grenadelauncher_livetime", "0.8", FCVAR_NOTIFY | FCVAR_CHEAT | FCVAR_REPLICATED);

#define OF_PIPEBOMB_TIMETOEXPLODE 2.0 // mayhaps change to a convar?

COFGrenadePipeBombProjectile::COFGrenadePipeBombProjectile()
{
	m_bTouched = false;
#ifdef GAME_DLL
	field_0x552 = true;
#endif
}

COFGrenadePipeBombProjectile::~COFGrenadePipeBombProjectile()
{
}

#ifdef GAME_DLL

void COFGrenadePipeBombProjectile::Spawn()
{
	if (m_iType == OF_GRENADELAUNCHERTYPE_STICKY)
	{
		SetModel("models/weapons/w_models/w_stickybomb.mdl");

		SetDetonateTimerLength(FLT_MAX); // the infamous 3402823466385288600000000000000000000 second long sticky :)
		//SetContextThink(&COFGrenadePipeBombProjectile::PreArmThink, gpGlobals->curtime, "PRE_ARM_THINK");
		//SetTouch()
	}
	else
	{
		SetModel("models/weapons/w_models/w_grenade_grenadelauncher.mdl");
		SetDetonateTimerLength(OF_PIPEBOMB_TIMETOEXPLODE);
		SetTouch(&COFGrenadePipeBombProjectile::PipeBombTouch);
	}

	SetCustomPipebombModel();

	BaseClass::Spawn();

	m_bTouched = false;
	m_flCreationTime = 0.0; //field_0x548 = 0.0;

	AddSolidFlags(FSOLID_TRIGGER);

	field_0x548 = 0.0;

	AddFlag(FL_GRENADE);
}

void COFGrenadePipeBombProjectile::Precache()
{
	PrecacheGibsForModel(PrecacheModel("models/weapons/w_models/w_stickybomb.mdl"));
	//PrecacheGibsForModel(PrecacheModel("models/weapons/w_models/w_stickybomb2.mdl")); // unused :v
	//PrecacheGibsForModel(PrecacheModel("models/weapons/w_models/w_stickybomb_d.mdl"));
	PrecacheGibsForModel(PrecacheModel("models/weapons/w_models/w_grenade_grenadelauncher.mdl"));

	// econ
	//PrecacheGibsForModel(PrecacheModel("models/weapons/w_models/w_cannonball.mdl"));
	//PrecacheModel("models/workshop/weapons/c_models/c_kingmaker_sticky/w_kingmaker_stickybomb.mdl");
	//PrecacheModel("models/workshop/weapons/c_models/c_quadball/w_quadball_grenade.mdl");

	PrecacheParticleSystem("stickybombtrail_blue");
	PrecacheParticleSystem("stickybombtrail_red");
	PrecacheScriptSound("Weapon_Grenade_Pipebomb.Bounce");
	//PrecacheScriptSound("Weapon_LooseCannon.BallImpact");
	BaseClass::Precache();
}

COFGrenadePipeBombProjectile *COFGrenadePipeBombProjectile::Create(const Vector &vecPos, const QAngle &angle, const Vector &vecSpeed, const AngularImpulse &angleImpulse, CBaseCombatCharacter *pPlayer, const COFWeaponInfo &weaponInfo, int iType, float flDamageMult)
{
	// cut this down as the other types are econ related
	// OFTODO: make these functions?
	int iProjectileType;
	switch (iType)
	{
	default:
	case OF_PROJECTILE_TYPE_PIPEBOMB:
		iProjectileType = OF_GRENADELAUNCHERTYPE_PIPE;
		break;
	case OF_PROJECTILE_TYPE_STICKYBOMB:
		iProjectileType = OF_GRENADELAUNCHERTYPE_STICKY;
		break;
	}

	char szProjectileName[128];
	switch (iProjectileType)
	{
	default:
	case OF_GRENADELAUNCHERTYPE_PIPE:
		V_snprintf(szProjectileName, sizeof(szProjectileName), "tf_projectile_pipe");
		break;
	case OF_GRENADELAUNCHERTYPE_STICKY:
		V_snprintf(szProjectileName, sizeof(szProjectileName), "tf_projectile_pipe_remote");
		break;
	}

	COFGrenadePipeBombProjectile *pProjectile = dynamic_cast<COFGrenadePipeBombProjectile*>(CBaseEntity::CreateNoSpawn(szProjectileName, vecPos, angle, pPlayer));
	if (pProjectile)
	{
		pProjectile->SetPipeBombMode(iProjectileType);
		DispatchSpawn(pProjectile);

		pProjectile->InitGrenade(vecSpeed, angleImpulse, pPlayer, weaponInfo);
		pProjectile->SetDamage(flDamageMult * pProjectile->GetDamage());
		pProjectile->SetInitialDamage(pProjectile->GetDamage());

		if (pProjectile->m_iType != OF_GRENADELAUNCHERTYPE_STICKY)
		{
			pProjectile->SetDamage(pProjectile->GetDamage() * 0.6);
		}

		pProjectile->ApplyLocalAngularVelocityImpulse(angleImpulse);

		// doesnt exist
		// if (pPlayer){ pProjectile->0xda = pPlayer->IsTruceValidForEnt(); }
	}

	return pProjectile;
}

void COFGrenadePipeBombProjectile::PipeBombTouch(CBaseEntity *pOther)
{
	if (pOther == GetThrower() || !pOther->IsSolid() || !pOther->IsSolidFlagSet(FSOLID_VOLUME_CONTENTS)) return;

	trace_t trace;
	Vector vecOrigin = GetAbsOrigin();
	Vector vecVelocity = GetAbsVelocity().Normalized();
	Vector vecCalc = (vecOrigin - vecVelocity * 32.0) + vecVelocity * 64.0;
	UTIL_TraceLine(vecOrigin, vecCalc, MASK_SOLID, this, COLLISION_GROUP_NONE, &trace);

	if (trace.fraction < 1.0 && trace.surface.flags & SURF_SKY)
	{
		UTIL_Remove(this);
		return;
	}

	// passtime :v
	/*
	iVar8 = __symbol_stub::_strcmp(pcVar17,"passtime_ball");
	if (iVar8 == 0)
	{
		pcVar25 = *(code **)(this->field_0x0 + 0x394);
		uVar16 = (**(code **)(this->field_0x0 + 0x3c4))(this);
	}
	*/

	if (m_bTouched) return;

	if (GetTeamNumber() && pOther->GetTeamNumber() == GetTeamNumber() && pOther->m_takedamage != DAMAGE_NO)
	{
		if (!pOther->IsPlayer())
		{
			// OFTODO: UNCOMMENT ME LATER!
			//CFuncRespawnRoom *pFunc = dynamic_cast<CFuncRespawnRoom*>(pOther);
			//if (pFunc)
			//{
			//	if (!pFunc->PointIsWithin(GetAbsOrigin())) return;
			//}

			// econ weapon, which seems to be the loose cannon
			//if ((this->field_0x530 == 3) ...
		}

		SetEnemy(pOther);

		m_flDamage = m_flInitialDamage;

		Explode(&trace, GetDamageType());
		return;
	}

	if (pOther->GetModelName() == AllocPooledString("models/props_vehicles/train_enginecar.mdl") && pOther->GetAbsVelocity().LengthSqr() > 1.0)
	{
		Explode(&trace, GetDamageType());
		return;
	}
}

void COFGrenadePipeBombProjectile::VPhysicsCollision(int index, gamevcollisionevent_t *pEvent)
{
	BaseClass::VPhysicsCollision(index,pEvent);

	int otherIndex = !index;
	CBaseEntity *pOther = pEvent->pEntities[otherIndex];

	if (!pOther) return;

	// this is for the loch and load for when it touches a surface it just, poofs
	/*  if (*(char *)((int)&this->field_0x544 + 2) != '\0')
	{
    (**(code **)(this->field_0x0 + 0x3ec))(this);
	(**(code **)(this->field_0x0 + 0x398))();
	return;
	*/

	if (m_iType == OF_GRENADELAUNCHERTYPE_PIPE)
	{
		// training mode dynamic props
		//PropDynamic_CollidesWithGrenades
		if (pOther->GetTeamNumber() && pOther->GetTeamNumber() != GetTeamNumber() && pOther->m_takedamage != DAMAGE_NO)
		{
			SetThink(&COFGrenadePipeBombProjectile::Detonate);
			SetNextThink(gpGlobals->curtime);
		}

		// attributesss

		m_bTouched = true;
		return;
	}

	bool bDynamicProp = (dynamic_cast<CDynamicProp*>(pOther) != NULL); // bVar19
	bool bCanStickToProp = (bDynamicProp && !pOther->HasSpawnFlags(SF_STICKY_MAY_NOT_STICK));

	if ((pOther->IsWorld() || (bCanStickToProp)) && gpGlobals->curtime > field_0x548)
	{
		m_bTouched = true;

		// what happens when a sticky hits a wall?
		// IT STICKS!
		g_PostSimulationQueue.QueueCall(VPhysicsGetObject(), &IPhysicsObject::EnableMotion, false );
		
		field_0x50c = true;

		pEvent->pInternalData->GetSurfaceNormal(field_0x510);
		field_0x510.Negate();

		// ATTRIBUTES HERE BE
	}
}

int COFGrenadePipeBombProjectile::OnTakeDamage(const CTakeDamageInfo &info)
{
	if (info.GetAttacker() && info.GetAttacker()->GetTeamNumber() != GetTeamNumber())
	{
		// field_0x550 + 2
		if (field_0x552 && m_bTouched && m_iType == OF_GRENADELAUNCHERTYPE_STICKY && (info.GetDamageType() & (DMG_BUCKSHOT | DMG_BLAST_SURFACE | DMG_SONIC | DMG_BLAST | DMG_BULLET)))
		{
			Vector vecForce = info.GetDamageForce();
			bool bDestroyPipe = false; // bVar16

			if (info.GetDamageType() & (DMG_BLAST_SURFACE | DMG_BULLET))
			{
				vecForce *= tf_grenade_forcefrom_bullet.GetFloat();
				bDestroyPipe = true;
			}

			if (info.GetDamageType() & (DMG_SONIC))
			{
				vecForce *= tf_grenade_forcefrom_bullet.GetFloat();
			}
			else if (info.GetDamageType() & (DMG_BUCKSHOT))
			{
				vecForce *= tf_grenade_forcefrom_buckshot.GetFloat();
				bDestroyPipe = true;
			}
			else if (info.GetDamageType() & (DMG_BLAST))
			{
				vecForce *= tf_grenade_forcefrom_blast.GetFloat();
			}

			if (bDestroyPipe)
			{
				// there's a huuuuge check here just for an achievment, ignored
				Fizzle();
				Detonate();
			}

			// line 171
			if (vecForce.LengthSqr() > (tf_pipebomb_force_to_move.GetFloat() * tf_pipebomb_force_to_move.GetFloat()))
			{
				if (VPhysicsGetObject())
				{
					VPhysicsGetObject()->EnableMotion(true);
				}

				CTakeDamageInfo newinfo = info;
				newinfo.SetDamageForce(vecForce);

				VPhysicsTakeDamage(info);

				field_0x548 = gpGlobals->curtime + tf_grenade_force_sleeptime.GetFloat();
				m_bTouched = false;
				field_0x50c = false;
				field_0x510.Zero();

				return 1;
			}
		}
	}

	return 0;
}

void COFGrenadePipeBombProjectile::IncrementDeflected()
{
	BaseClass::IncrementDeflected();

	if (GetDeflected() && m_iType == OF_GRENADELAUNCHERTYPE_STICKY)
	{
		field_0x54c = gpGlobals->curtime + tf_pipebomb_deflect_reset_time.GetFloat();
	}

	int iTeam = GetTeamNumber();

	if (GetDeflectOwner() && GetDeflectOwner()->IsPlayer())
	{
		iTeam = GetDeflectOwner()->GetTeamNumber();
	}

	if (m_iType != OF_GRENADELAUNCHERTYPE_STICKY)
	{
		switch (iTeam)
		{
		case OF_TEAM_RED:
			m_nSkin = 0;
			break;
		case OF_TEAM_BLUE:
			m_nSkin = 1;
			break;
		}
	}
}

void COFGrenadePipeBombProjectile::DetonateThink()
{
	BaseClass::DetonateThink();

	if (gpGlobals->curtime >= field_0x54c && m_iType == OF_GRENADELAUNCHERTYPE_STICKY)
	{
		SetDeflected(0);
		SetDeflectOwner(NULL);
	}

	// there's a weird feature here where if you create stickies during the setup phase that have crits from the kritzkreg
	// they'll destroy themselves if the medic doesnt exist, weird but true!
}

void COFGrenadePipeBombProjectile::SetPipeBombMode(int iType)
{
	m_iType = iType;
}

void COFGrenadePipeBombProjectile::Detonate()
{
	// here be attributes

	if (m_bFizzle)
	{
		g_pEffects->Sparks(GetAbsOrigin(), 1, 2);
		Destroy();

		//CreatePipeBombGibs();

		return;
	}

	BaseClass::Detonate();
}

void COFGrenadePipeBombProjectile::CreatePipeBombGibs()
{
	CPVSFilter filter(GetAbsOrigin());
	UserMessageBegin(filter, "CheapBreakModel");
	MessageWriteShort(GetModelIndex());
	MessageWriteVec3Coord(GetAbsOrigin());
	MessageEnd();
}

void COFGrenadePipeBombProjectile::Fizzle()
{
	m_bFizzle = true;
}
#else

void COFGrenadePipeBombProjectile::OnDataChanged(DataUpdateType_t updateType)
{
	BaseClass::OnDataChanged(updateType);

	if (updateType == DATA_UPDATE_CREATED)
	{
		m_flCreationTime = gpGlobals->curtime;
		m_bPulsed = false;

		// OFTODO: uncomment me later!
		//CTFPipeBombLauncher *pLauncher = dynamic_cast<CTFPipeBombLauncher*>(GetLauncher()); // capalatize the B :lilacstare:
		//if (pLauncher)
		//{
			//pLauncher->AddPipeBomb(this);
		//}

		// glowing stuff here
		// likely for the scottish resistance

		//CreateTrailParticles();
	}

	if (field_0x8f4 != GetDeflected())
	{
		//CreateTrailParticles();
	}

	field_0x8f4 = GetDeflected();
}

const char *COFGrenadePipeBombProjectile::GetTrailParticleName()
{
	int iTeam = GetTeamNumber();

	// client: 
	// m_iType = 0x8dc
	// m_iDeflected = 0x8cc

	if (GetDeflected() && m_iType != OF_GRENADELAUNCHERTYPE_STICKY && GetDeflectOwner() && GetDeflectOwner()->IsPlayer())
	{
		iTeam = GetDeflectOwner()->GetTeamNumber();
	}

	bool bIsSticky = (m_iType == OF_GRENADELAUNCHERTYPE_STICKY);

	switch (iTeam)
	{
	default:
	case OF_TEAM_RED:
		return (bIsSticky) ? "stickybombtrail_red" : "pipebombtrail_red";
	case OF_TEAM_BLUE:
		return (bIsSticky) ? "stickybombtrail_blue" : "pipebombtrail_blue";
	}
}

void COFGrenadePipeBombProjectile::Simulate()
{
	BaseClass::Simulate();

	if (m_iType != OF_GRENADELAUNCHERTYPE_STICKY && !m_bPulsed)
	{
		if (GetLiveTime() <= gpGlobals->curtime - m_flCreationTime)
		{
			const char *pszPulse;

			switch (GetTeamNumber())
			{
			default:
			case OF_TEAM_RED:
				pszPulse = "stickybomb_pulse_red";
				break;
			case OF_TEAM_BLUE:
				pszPulse = "stickybomb_pulse_blue";
				break;
			}

			ParticleProp()->Create(pszPulse, PATTACH_ABSORIGIN_FOLLOW, NULL);

			m_bPulsed = true; // field_0x8e8
			//if () Detonate(); // why is this called on the client???
		}
	}
}

int COFGrenadePipeBombProjectile::DrawModel(int flags)
{
	if (gpGlobals->curtime < m_flCreationTime + 0.1) return 0;

	return BaseClass::DrawModel(flags);
}

#endif

int COFGrenadePipeBombProjectile::GetWeaponID()
{
	//0x5b = 91 - econ weapon

	//OF_WEAPON_GRENADE_PIPEBOMB OF_WEAPON_GRENADE_DEMOMAN

	if (m_iType == OF_GRENADELAUNCHERTYPE_STICKY)
	{
		return OF_WEAPON_GRENADE_PIPEBOMB;
	}

	return OF_WEAPON_GRENADE_DEMOMAN;
}

int COFGrenadePipeBombProjectile::GetDamageType()
{
	int iDamageType = BaseClass::GetDamageType();
	if (m_iType == OF_GRENADELAUNCHERTYPE_STICKY && gpGlobals->curtime - m_flCreationTime < 5.0)
	{
		iDamageType |= DMG_USEDISTANCEMOD;
	}

	return iDamageType;
}

void COFGrenadePipeBombProjectile::UpdateOnRemove()
{
	if (GetLauncher())
	{
		//CTFPipebombLauncher *pLauncher = dynamic_cast<CTFPipebombLauncher*>(GetLauncher());
		//pLauncher->DeathNotice(this);
	}

	BaseClass::UpdateOnRemove();
}

float COFGrenadePipeBombProjectile::GetLiveTime()
{
	return tf_grenadelauncher_livetime.GetFloat();
}