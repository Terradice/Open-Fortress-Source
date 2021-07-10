// ========= Copyright Open Fortress Developers, CC-BY-NC-SA ============
// Purpose: Implementation of CTFWeaponBaseGrenadeProj
// Author(s): Cherry!
//

#pragma once

#include "basegrenade_shared.h"
#include "networkstringtabledefs.h"

#ifdef CLIENT_DLL
	#define COFWeaponBaseGrenadeProj C_OFWeaponBaseGrenadeProj
#endif

class COFWeaponBaseGrenadeProj : public CBaseGrenade
{
public:

	DECLARE_CLASS(COFWeaponBaseGrenadeProj, CBaseGrenade);
	DECLARE_NETWORKCLASS();

	COFWeaponBaseGrenadeProj();
	virtual ~COFWeaponBaseGrenadeProj();

	virtual void Precache();
	virtual void Spawn();

	#ifdef GAME_DLL

	virtual void InitGrenade(const Vector &vecSpeed, const AngularImpulse &angleImpulse, CBaseCombatCharacter *pPlayer, const COFWeaponInfo &weaponInfo);
	virtual void InitGrenade(const Vector &vecSpeed, const AngularImpulse &angleImpulse, CBaseCombatCharacter *pPlayer, int iDamage, float flDamageRadius);

	virtual void DetonateThink();
	virtual void Detonate();
	virtual void Explode(trace_t *pTrace, int bitsDamageType);
	virtual int OnTakeDamage(const CTakeDamageInfo &info);
	virtual void SetDetonateTimerLength(float flTime);
	virtual void ResolveFlyCollisionCustom(trace_t &trace, Vector &vec);
	virtual void Destroy(bool bBlinkOut = true, bool bBreakRocket = false);
	virtual void VPhysicsUpdate(IPhysicsObject *pPhysics);
	virtual bool IsDestroyable(bool param_1);
	virtual bool IsAllowedToExplode() { return true; }
	virtual float GetDetonateTime() { return m_flDetonateTime; }

	virtual void SetEnemy(CBaseEntity *pEntity) { m_pEnemy = pEntity; }
	CBaseEntity *GetEnemy() { return m_pEnemy; } // 0x520

	#else

	virtual void OnDataChanged(DataUpdateType_t updateType);

	#endif

	virtual CBaseEntity *GetDeflectOwner() { return m_hDeflectOwner; }
	virtual void SetDeflectOwner(CBaseEntity *pDeflectOwner) { m_hDeflectOwner = pDeflectOwner; }
	virtual int GetDeflected() { return m_iDeflected; }
	virtual void IncrementDeflected() { m_iDeflected++; }
	virtual void SetDeflected(int iDeflected) { m_iDeflected = iDeflected; }
	CBaseEntity *GetLauncher() { return m_hLauncher; }
	virtual int GetBaseProjectileType() const { return OF_PROJECTILE_TYPE_NONE; }
	virtual void SetLauncher(CBaseEntity *pPlayer);
	virtual float GetShakeAmplitude() { return 10.0; }
	virtual float GetShakeRadius() { return 300.0; }
	virtual int GetCustomParticleIndex() { return INVALID_STRING_INDEX; }
	virtual int GetWeaponID() { return WEAPON_NONE; }
	virtual int GetDamageType();
	virtual int GetDamageCustom() { return DMG_GENERIC; }
	virtual float GetDamageRadius();
	void SetCritical(bool bCritical) { m_bCritical = bCritical; }

private:

	CNetworkVector(m_vInitialVelocity); // 1268 = 0x4F4

	CNetworkHandle(CBaseEntity, m_hLauncher); // 1280 = 0x500

	CNetworkVar(bool, m_bCritical); // 1317 = 0x525

	CNetworkVar(int, m_iDeflected); // 1284 = 0x504
	CNetworkHandle(CBaseEntity, m_hDeflectOwner); // 1288 = 0x508

#ifdef CLIENT_DLL

	float m_flSpawnTime;

#else

protected:

	Vector field_0x510;
	bool field_0x50c;

private:

	CBaseEntity *m_pEnemy; // 0x520
	float field_0x528;
	float m_flDetonateTime; // 0x51C = 1308
	bool field_0x524;

#endif

};