// ========= Copyright Open Fortress Developers, CC-BY-NC-SA ============
// Purpose: Implementation of CTFWeaponBaseGrenadeProj
// Author(s): Cherry!
//

#include "cbase.h"
#include "of_shareddefs.h"
#include "of_weapon_base.h"
#include "of_weapon_base_grenadeproj.h"
#include "of_gamerules.h"

#ifdef GAME_DLL

#include "Sprite.h"
#include "soundent.h"

#endif

// ----------------------------------------------------------------------------- //
// Data tables
// ----------------------------------------------------------------------------- //

IMPLEMENT_NETWORKCLASS_ALIASED(OFWeaponBaseGrenadeProj, DT_OFWeaponBaseGrenadeProj)

BEGIN_NETWORK_TABLE(COFWeaponBaseGrenadeProj, DT_OFWeaponBaseGrenadeProj)
#ifdef CLIENT_DLL
	RecvPropVector(RECVINFO_NAME(m_vInitialVelocity, m_vInitialVelocity)),
	RecvPropBool(RECVINFO(m_bCritical)),
	RecvPropVector(RECVINFO_NAME(m_vecNetworkOrigin, m_vecOrigin)),
	RecvPropQAngles(RECVINFO_NAME(m_angNetworkAngles, m_angRotation)),
	RecvPropInt(RECVINFO(m_iDeflected)),
	RecvPropEHandle(RECVINFO(m_hDeflectOwner)),
#else
	SendPropVector(SENDINFO(m_vInitialVelocity), 20, 0, -3000.0, 3000),
	SendPropBool(SENDINFO(m_bCritical)),
	SendPropExclude("DT_BaseEntity", "m_vecOrigin"),
	SendPropExclude("DT_BaseEntity", "m_angRotation"),
	SendPropVector(SENDINFO(m_vecOrigin), -1, SPROP_CHANGES_OFTEN | SPROP_COORD_MP_INTEGRAL),
	SendPropQAngles(SENDINFO(m_angRotation), 6, SPROP_CHANGES_OFTEN),
	SendPropInt(SENDINFO(m_iDeflected), 4),
	SendPropEHandle(SENDINFO(m_hDeflectOwner)),
#endif
END_NETWORK_TABLE()

COFWeaponBaseGrenadeProj::COFWeaponBaseGrenadeProj()
{
#ifdef GAME_DLL
	field_0x50c = false;
#endif
}

COFWeaponBaseGrenadeProj::~COFWeaponBaseGrenadeProj()
{

}

void COFWeaponBaseGrenadeProj::Precache()
{
	BaseClass::Precache();
#ifdef GAME_DLL
	PrecacheModel("sprites/light_glow02_noz.vmt", true);
	PrecacheParticleSystem("critical_grenade_blue");
	PrecacheParticleSystem("critical_grenade_red");
	PrecacheParticleSystem("ExplosionCore_Wall_Jumper");
#endif
}

void COFWeaponBaseGrenadeProj::Spawn()
{
#ifdef CLIENT_DLL
	// Client
	m_flSpawnTime = gpGlobals->curtime;
	BaseClass::Spawn();
	AddFlag(FL_GRENADE);

#else
	// Server
	BaseClass::Spawn();
	AddSolidFlags(FSOLID_NOT_STANDABLE);
	SetSolid(SOLID_BBOX);
	AddEffects(EF_NOSHADOW);

	UTIL_SetSize(this, Vector(-2.0,-2.0,-2.0), Vector(2.0,2.0,2.0));
	SetCollisionGroup(OF_COLLISION_GROUP_UNKNOWN20);

	VPhysicsInitNormal(SOLID_BBOX, 0, false);

	m_takedamage = DAMAGE_EVENTS_ONLY;

	ChangeTeam(GetThrower() ? GetThrower()->GetTeamNumber() : TEAM_UNASSIGNED);

	m_nSkin = (GetTeamNumber() == OF_TEAM_BLUE);

	field_0x528 = gpGlobals->curtime + 0.25;

	SetThink(&COFWeaponBaseGrenadeProj::DetonateThink);
	SetNextThink(gpGlobals->curtime);

#endif
}

#ifdef GAME_DLL
	
void COFWeaponBaseGrenadeProj::DetonateThink()
{
	if (!IsInWorld())
	{
		Remove();
		return;
	}

	if (m_flDetonateTime < gpGlobals->curtime)
	{
		Detonate();
		return;
	}

	SetNextThink(gpGlobals->curtime + 0.2);
}

void COFWeaponBaseGrenadeProj::Detonate()
{
	SetThink(NULL);

	trace_t tr;
	Vector vecSpot = GetAbsOrigin() + Vector(0, 0, 8);
	UTIL_TraceLine(vecSpot, vecSpot + Vector(0, 0, -32), MASK_SHOT_HULL, this, COLLISION_GROUP_NONE, &tr);

	Explode(&tr, GetDamageType());

	if (GetShakeAmplitude())
	{
		UTIL_ScreenShake(GetAbsOrigin(), GetShakeAmplitude(), 150.0, 1.0, GetShakeRadius(), SHAKE_START);
	}
}

void COFWeaponBaseGrenadeProj::Explode(trace_t *pTrace, int bitsDamageType)
{
	//if (InNoGrenadeZone())
	//{
	//	Destroy();
	//	return;
	//}

	SetModelName(NULL_STRING);
	AddSolidFlags(FSOLID_NOT_SOLID);

	m_takedamage = DAMAGE_NO;

	// Pull out of the wall a bit
	if (pTrace->fraction != 1.0)
	{
		SetAbsOrigin(pTrace->endpos + (pTrace->plane.normal * 0.6));
	}

	CSoundEnt::InsertSound(SOUND_COMBAT, GetAbsOrigin(), BASEGRENADE_EXPLOSION_VOLUME, 3.0);

	Vector vecOrigin = GetAbsOrigin();
	CPVSFilter filter(vecOrigin);

	// halloween, attributes, and mvm stuff here

	if (field_0x50c)
	{
		if (pTrace->m_pEnt && pTrace->m_pEnt->IsPlayer())
		{
			//TE_TFExplosion(filter, 0.0, vecOrigin, field_0x510, GetWeaponID(), )
		}
		else
		{
			//TE_TFExplosion(filter, 0.0, vecOrigin, field_0x510, GetWeaponID(), )
		}
	}
	else
	{
		if (pTrace->m_pEnt && pTrace->m_pEnt->IsPlayer())
		{
			
		}
		else
		{
			//TE_TFExplosion(filter, 0.0, vecOrigin, pTrace->plane.normal, GetWeaponID(), )
			
		}
	}

	Vector vecThrowerOrigin = GetThrower() ? GetThrower()->GetAbsOrigin() : vec3_origin;
	CTakeDamageInfo info(this, GetThrower(), m_hLauncher, GetBlastForce(), GetAbsOrigin(), m_flDamage, bitsDamageType, GetDamageCustom(), &vecThrowerOrigin);

	// CTFRadiusDamageInfo is FILLED with attribute stuff, and RadiusDamage has comments explaining
	// that a tf2 specific radius function is needed, so my guess this is how it was done in 2008 tf2
	RadiusDamage(info, vecOrigin, GetDamageRadius(), OF_CLASS_UNDEFINED, NULL);

	if (pTrace->m_pEnt && !pTrace->m_pEnt->IsPlayer())
	{
		UTIL_DecalTrace(pTrace, "Scorch");
	}

	//if (GetEnemy() && GetThrower() && GetEnemy()->IsPlayer())
	//{
		//CBaseProjectile::RecordEnemyPlayerHit() - this doesnt exist?
	//}

	SetThink(&BaseClass::SUB_Remove);
	SetTouch(NULL); // 0xe4 / 0xe0

	AddEffects(EF_NODRAW);
	SetAbsVelocity(vec3_origin);
	SetNextThink(gpGlobals->curtime);
}

int COFWeaponBaseGrenadeProj::OnTakeDamage(const CTakeDamageInfo &info)
{
	CTakeDamageInfo newinfo = info;

	if (info.GetDamageType() & DMG_BLAST) // m_bitsDamageType = 0x3c
	{
		newinfo.ScaleDamageForce(0.05);
	}

	return BaseClass::OnTakeDamage(newinfo);
}

void COFWeaponBaseGrenadeProj::SetDetonateTimerLength(float flTime)
{
	m_flDetonateTime = gpGlobals->curtime + flTime;
}

void COFWeaponBaseGrenadeProj::ResolveFlyCollisionCustom(trace_t &trace, Vector &vecVelocity)
{
	float flElasticity = 1.0;

	if (trace.m_pEnt && trace.m_pEnt->IsPlayer())
	{
		flElasticity = 0.3;
	}

	flElasticity = flElasticity * GetElasticity();
	flElasticity = clamp(flElasticity, 0.0, 0.9);

	Vector vecAbsVelocity;
	PhysicsClipVelocity(GetAbsVelocity(), trace.plane.normal, vecAbsVelocity, 2.0);

	vecAbsVelocity *= flElasticity; // local_1c, local_18, local_14

	// Get the total velocity (player + conveyors, etc.)
	VectorAdd(vecAbsVelocity, GetBaseVelocity(), vecVelocity);
	float flSpeed = DotProduct(vecVelocity, vecVelocity); // fVar4

	// Stop if on ground.
	if (trace.plane.normal.z > 0.7) // Floor
	{
		// Verify that we have an entity.
		CBaseEntity *pEntity = trace.m_pEnt; // this_00
		Assert(pEntity);

		SetAbsVelocity(vecAbsVelocity);

		if ((30 * 30) > flSpeed)
		{
			if (pEntity->IsStandable())
			{
				SetGroundEntity(pEntity);
			}

			SetAbsVelocity(vec3_origin);
			SetLocalAngularVelocity(vec3_angle);

			QAngle angle;
			VectorAngles(trace.plane.normal, angle);

			angle.z = random->RandomFloat(0, 360.0);

			SetAbsAngles(angle);
		}
		else
		{
			// taken from CBaseEntity::ResolveFlyCollisionBounce
			Vector vecDelta = GetBaseVelocity() - vecAbsVelocity;
			Vector vecBaseDir = GetBaseVelocity();
			VectorNormalize(vecBaseDir);
			float flScale = vecDelta.Dot(vecBaseDir);

			VectorScale(vecAbsVelocity, (1.0f - trace.fraction) * gpGlobals->frametime, vecVelocity);
			VectorMA(vecVelocity, (1.0f - trace.fraction) * gpGlobals->frametime, GetBaseVelocity() * flScale, vecVelocity);
			PhysicsPushEntity(vecVelocity, &trace);
		}
	}
	else
	{
		if ((30 * 30) > flSpeed)
		{
			SetAbsVelocity(vec3_origin);
			SetLocalAngularVelocity(vec3_angle);
		}
		else
		{
			SetAbsVelocity(vecAbsVelocity);
		}
	}

	BounceSound();
}

void COFWeaponBaseGrenadeProj::Destroy(bool bBlinkOut, bool bBreakRocket)
{
	if (bBreakRocket)
	{
		CPVSFilter filter(GetAbsOrigin());
		UserMessageBegin(filter, "BreakModelRocketDud");
		MessageWriteShort(GetModelIndex());
		MessageWriteVec3Coord(GetAbsOrigin());
		MessageWriteAngles(GetAbsAngles());
		MessageEnd();
	}

	SetThink(&BaseClass::SUB_Remove);
	SetNextThink(gpGlobals->curtime);
	SetTouch(NULL);
	AddEffects(EF_NODRAW);

	if (bBlinkOut)
	{
		CSprite *pSprite = CSprite::SpriteCreate("sprites/light_glow02_noz.vmt", GetAbsOrigin(), true); // why not animate it?
		if (pSprite)
		{
			pSprite->SetTransparency(kRenderGlow, 255, 255, 255, 255, kRenderFxFadeFast);
			pSprite->SetThink(&BaseClass::SUB_Remove);
			pSprite->SetNextThink(gpGlobals->curtime + 1.0);
		}
	}
}

class CTraceFilterCollisionGrenades : public CTraceFilterEntitiesOnly
{
public:

	DECLARE_CLASS_NOBASE(CTraceFilterCollisionGrenades);

	CTraceFilterCollisionGrenades(const IHandleEntity *passentity1, const IHandleEntity *passentity2)
		: m_pPassEnt1(passentity1), m_pPassEnt2(passentity2)
	{
	}

	// https://gist.github.com/CherrySodaPop/f3326a48c1994e0d0799b199a6823405
	virtual bool ShouldHitEntity(IHandleEntity *pHandleEntity, int contentsMask)
	{
		if (!PassServerEntityFilter(pHandleEntity, m_pPassEnt1)) return false;

		CBaseEntity *pEntity = EntityFromEntityHandle(pHandleEntity);
		if (!pEntity) return true;

		if (pEntity == m_pPassEnt2) return false;

		if (pEntity->GetCollisionGroup() == COLLISION_GROUP_NONE) return false;

		if (pEntity->GetCollisionGroup() == COLLISION_GROUP_DEBRIS) return false;

		if (pEntity->GetCollisionGroup() == OF_COLLISION_GROUP_UNKNOWN20) return false;

		if (pEntity->GetCollisionGroup() == OF_COLLISION_GROUP_ROCKETS) return false;

		if (pEntity->GetCollisionGroup() == OF_COLLISION_GROUP_UNKNOWN25) return false;

		return true;
	}

protected:

	const IHandleEntity *m_pPassEnt1; // 0x4
	const IHandleEntity *m_pPassEnt2; // 0x8
};

void COFWeaponBaseGrenadeProj::VPhysicsUpdate(IPhysicsObject *pPhysics)
{
	BaseClass::VPhysicsUpdate(pPhysics);

	Vector vecVelocity; // local_7c
	AngularImpulse angularVelocity; // local_8c
	pPhysics->GetVelocity(&vecVelocity, &angularVelocity);

	Vector vecOrigin = GetAbsOrigin();

	CTraceFilterCollisionGrenades filter(this, GetThrower());

	// mvm cringe
	//if (OFGameRules() && OFGameRules()->GameModeUsesUpgrades())

	// 1107296257 1073741824 , 33554433 33554432 , 1
	trace_t trace;

	UTIL_TraceLine(vecOrigin, vecOrigin + vecVelocity * gpGlobals->frametime, CONTENTS_HITBOX | CONTENTS_MONSTER | CONTENTS_SOLID, &filter, &trace);

	// there's an function here the gets the opposing team for the team your currently on
	// that limits your attacks to said team, so instead just check if your not on that team
	// this also explains why console created pills wont damage you! - cherry
	bool bEnemy = trace.m_pEnt && trace.m_pEnt->GetTeamNumber() != GetTeamNumber();
	bool bFriend = trace.m_pEnt && trace.m_pEnt->GetTeamNumber() == GetTeamNumber() && CanCollideWithTeammates();

	if (bEnemy && trace.m_pEnt->IsCombatItem())
	{
		if (IsAllowedToExplode())
		{
			Explode(&trace, GetDamageType());
		}
		else
		{
			if (pPhysics)
			{
				Vector vecVelocity;
				pPhysics->GetVelocity(&vecVelocity, NULL);
				vecVelocity *= -0.2;
				pPhysics->SetVelocity(&vecVelocity, NULL);
			}
		}

		return;
	}

	if (trace.startsolid)
	{
		if (bEnemy)
		{
			Touch(trace.m_pEnt);
		}
		else
		{
			if (!field_0x524 && bEnemy && pPhysics)
			{
				Vector vecVelocity;
				pPhysics->GetVelocity(&vecVelocity, NULL);
				vecVelocity *= -0.2;
				pPhysics->SetVelocity(&vecVelocity, NULL);
			}
		}

		field_0x524 = true;
		return;
	}

	field_0x524 = false;

	// this begins at line 129
	if (trace.DidHit())
	{
		Touch(trace.m_pEnt);

		if (bFriend || bEnemy)
		{
			vecVelocity = (trace.plane.normal * -2.0 * DotProduct(vecVelocity, trace.plane.normal) + vecVelocity) * GetElasticity();

			if (bEnemy)
			{
				vecVelocity *= 0.5;
			}

			angularVelocity *= -0.5;

			pPhysics->SetVelocity(&vecVelocity, &angularVelocity);
		}

		return;
	}
}

void COFWeaponBaseGrenadeProj::InitGrenade(const Vector &vecSpeed, const AngularImpulse &angleImpulse, CBaseCombatCharacter *pPlayer, const COFWeaponInfo &weaponInfo)
{
	InitGrenade(vecSpeed, angleImpulse, pPlayer, weaponInfo.m_WeaponModeInfo[OF_WEAPON_MODE_PRIMARY].m_iDamage, weaponInfo.m_flDamageRadius);
}

void COFWeaponBaseGrenadeProj::InitGrenade(const Vector &vecSpeed, const AngularImpulse &angleImpulse, CBaseCombatCharacter *pPlayer, int iDamage, float flDamageRadius)
{
	SetOwnerEntity(this);
	SetThrower(pPlayer);

	m_vInitialVelocity = vecSpeed;

	// when you convert the hex values to floats it wont equal exactly 0.4 or 0.5, 
	// here's something giving an explanation on why https://www.exploringbinary.com/why-0-point-1-does-not-exist-in-floating-point/
	SetGravity(0.4); // 0x288 = 0x3ecccccd
	SetFriction(0.2); // 0x28c = 0x3e4ccccd
	SetElasticity(0.5); // 0x290 = 0x3ee66666

	SetDamage(float(iDamage));
	SetDamageRadius(flDamageRadius);

	ChangeTeam(pPlayer ? pPlayer->GetTeamNumber() : TEAM_UNASSIGNED);

	IPhysicsObject *pObj = VPhysicsGetObject(); // m_pPhysicsObject = 0x208
	if (pObj)
	{
		pObj->AddVelocity(&vecSpeed, &angleImpulse);
	}
}

bool COFWeaponBaseGrenadeProj::IsDestroyable(bool param_1)
{
	if (!param_1) return (field_0x528 < gpGlobals->curtime);

	return true;
}

#else

void COFWeaponBaseGrenadeProj::OnDataChanged(DataUpdateType_t updateType)
{
	BaseClass::OnDataChanged(updateType);

	if (updateType == DATA_UPDATE_CREATED)
	{
		CInterpolatedVar< Vector > &vecInterpolator = GetOriginInterpolator();

		vecInterpolator.ClearHistory(); // 0xe = m_VarHistory

		float flTime = GetLastChangeTime(LATCH_SIMULATION_VAR);

		Vector vecOrigin = GetLocalOrigin() - m_vInitialVelocity; // m_vInitialVelocity = 0x8bc
		vecInterpolator.AddToHead(flTime - 1.0, &vecOrigin, false);

		vecOrigin = GetLocalOrigin();
		vecInterpolator.AddToHead(flTime, &vecOrigin, false);
	}
}

#endif

void COFWeaponBaseGrenadeProj::SetLauncher(CBaseEntity *pPlayer)
{
	m_hLauncher = pPlayer;
	BaseClass::SetLauncher(pPlayer);
}

int COFWeaponBaseGrenadeProj::GetDamageType()
{
	int iDamageType = g_aWeaponDamageTypes[GetWeaponID()];

	if (m_bCritical)
	{
		iDamageType |= DMG_CRIT;
	}

	return iDamageType;
}

float COFWeaponBaseGrenadeProj::GetDamageRadius()
{
	// attributes - ignore
	return m_DmgRadius; // field_0x4d4
}