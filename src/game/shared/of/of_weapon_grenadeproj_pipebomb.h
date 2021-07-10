// ========= Copyright Open Fortress Developers, CC-BY-NC-SA ============
// Purpose: Implementation of CTFGrenadePipeBombProjectile
// Author(s): Cherry!
//

#pragma once

#include "of_weapon_base_grenadeproj.h"

#ifdef CLIENT_DLL
	#define COFGrenadePipeBombProjectile C_OFGrenadePipeBombProjectile
#endif

class COFGrenadePipeBombProjectile : public COFWeaponBaseGrenadeProj
{
public:
	
	DECLARE_CLASS(COFGrenadePipeBombProjectile, COFWeaponBaseGrenadeProj);
	DECLARE_NETWORKCLASS();

	COFGrenadePipeBombProjectile();
	~COFGrenadePipeBombProjectile();

	virtual void SetCustomPipebombModel() { return; }
	virtual int GetWeaponID();
	virtual int GetDamageType();
	virtual void UpdateOnRemove();
	virtual float GetLiveTime(); // filled with attributes, cut down to just get the convar

#ifdef GAME_DLL

	virtual void Spawn();
	virtual void Precache();

	static COFGrenadePipeBombProjectile *Create(const Vector &vecPos, const QAngle &angle, const Vector &vecSpeed, const AngularImpulse &angleImpulse, CBaseCombatCharacter *pPlayer, const COFWeaponInfo &weaponInfo, int iType, float flDamageMult);

	virtual void PipeBombTouch(CBaseEntity *pOther); // i made the B in bomb in uppercase because i was going insane from the fact it wasnt
	virtual void VPhysicsCollision(int index, gamevcollisionevent_t *pEvent);
	virtual int OnTakeDamage(const CTakeDamageInfo &info);
	virtual void IncrementDeflected();
	virtual void DetonateThink();
	// OFTODO:
	//PreArmThink();
	//ArmThink();
	//Deflected();
	//GetDamageCustom();
	//UpdateTransmitState();
	//ShouldTransmit();
	//GetDamageRadius();

	virtual void SetPipeBombMode(int iType); // i made the B in bomb in uppercase because i was going insane from the fact it wasnt
	virtual void SetInitialDamage(float flDamage) { m_flInitialDamage = flDamage; }
	virtual bool IsDeflectable() { return true; }
	virtual bool ShouldMiniCritOnReflect() { return (m_iType == OF_GRENADELAUNCHERTYPE_PIPE); }
	virtual void BounceSound() { BaseClass::EmitSound("Weapon_Grenade_Pipebomb.Bounce"); }
	virtual void Detonate();
	virtual void CreatePipeBombGibs(); // i made the B in bomb in uppercase because i was going insane from the fact it wasnt
	virtual void Fizzle();

#else

	virtual void OnDataChanged(DataUpdateType_t updateType);
	virtual const char *GetTrailParticleName();
	virtual void Simulate();
	virtual int DrawModel(int flags);

#endif

private:

	CNetworkVar(bool, m_bTouched); // 1325 = 0x52c + 1
	CNetworkVar(int, m_iType); // 1328 = 0x530
	//float field_0x534; // 0x534
	float m_flCreationTime; // field_0x8e0

#ifdef GAME_DLL

	bool m_bFizzle; // field_0x545
	float m_flInitialDamage;
	float field_0x548;
	bool field_0x552;
	float field_0x54c;
#else

	bool m_bPulsed; // field_0x8e8
	int field_0x8f4;

#endif
};