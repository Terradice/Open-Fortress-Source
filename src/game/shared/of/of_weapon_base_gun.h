// ========= Copyright Open Fortress Developers, CC-BY-NC-SA ============
// Purpose: COFWeaponBaseGun is the root of most ranged weapons
// Author(s): KaydemonLP
//
#pragma once

#include "of_weapon_base.h"

#if defined( CLIENT_DLL )
    #define COFWeaponBaseGun C_OFWeaponBaseGun
#endif

class COFWeaponBaseGun: public COFWeaponBase
{
    DECLARE_CLASS(COFWeaponBaseGun, COFWeaponBase);
    DECLARE_NETWORKCLASS();
    DECLARE_PREDICTABLE();
	
#ifdef GAME_DLL
	DECLARE_DATADESC();
#endif

    //NOTE: I'm basing this class interface entirely on the serverside vtable, and so..
    //OFTODO: Compare COFWeaponBaseGun members to client.dylib CTFWeaponBaseGun

    COFWeaponBaseGun();
	~COFWeaponBaseGun();

	//virtual bool CanBeSelected();
	//virtual bool HasPrimaryAmmo();
	//virtual bool CanDeploy();
	virtual bool Holster(CBaseCombatWeapon *pSwitchingTo);

	virtual void PrimaryAttack();
	virtual void SecondaryAttack();
	
	// What is this used for??
	// Re: Apparently there's a "last_shot_crits" attribute, dont think anything uses it - Kay
	//virtual bool HasLastShotCritical();
	
	virtual void DoFireEffects();

	// OFTODO: Placeholder for now, look into what makes this be false sometimes
	virtual bool ShouldDoMuzzleFlash() { return true; };

	virtual int GetWeaponProjectileType() const;

	virtual CBaseEntity *FireProjectile( COFPlayer *pPlayer );
	/*
	virtual void RemoveProjectileAmmo( CTFPlayer *pPlayer );
	virtual void ModifyProjectile( CBaseEntity *pEnt );
	*/
	virtual void FireBullet( COFPlayer *pPlayer );
	
	// OFTODO: Figure out what the types are
	virtual CBaseEntity *FirePipeBomb(COFPlayer *pPlayer, int iType);
	virtual CBaseEntity *FireRocket(COFPlayer *pPlayer, int iType);
	// OFTODO: Ditto
	//virtual void FireArrow( CTFPlayer *pPlayer, int ProjectileType_t );
	
	//virtual void FireJar( CTFPlayer *pPlayer );
	//virtual void FireFlameRocket( CTFPlayer *pPlayer );
	// OFTODO: Figure out what the magic bool does
	//virtual void FireEnergyBall( CTFPlayer *pPlayer, bool bMagic );
	
	virtual float GetWeaponSpread();

	// virtual char GetCustomProjectileModel( CAttribute_String *szParam ); // IGNORE, ECON HELL - cherry

	virtual float GetProjectileSpeed();
	virtual float GetProjectileGravity();
	//virtual float GetProjectileSpread();
	
	virtual int GetAmmoPerShot();
	virtual int GetProjectileDamage();
	
	virtual bool ShouldPlayFireAnim();
	/*
	virtual void ZoomIn();
	virtual void ZoomOut();
	*/
	virtual void PlayWeaponShootSound();
	// OFTODO: Placeholder for now, i think only attributes affect this so it will stay hardcoded as true for now - Kay
	virtual bool ShouldRemoveDisguiseOnPrimaryAttack() const;
};