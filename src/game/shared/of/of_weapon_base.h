// ========= Copyright Open Fortress Developers, CC-BY-NC-SA ============
// Purpose: COFWeaponBase is the root of all OF weapons
// Author(s): Nopey, Fenteale
//
#pragma once

#include "basecombatweapon_shared.h"
#include "of_weapon_parse.h"
#include "econ/ihasowner.h"
#include "predictable_entity.h"
#include "npcevent.h"

#ifdef CLIENT_DLL
#define COFPlayer C_OFPlayer
#endif

//--------------------------------------------------------------------------------------------------------
//
// Weapon IDs for all OF Game weapons
//
typedef enum
{
	// OFTODO: there might be some econ weapons here and cut weapons, they are only for refrence
	//			to make sure the other weapon ids are correct, remove them in the future! - cherry

    WEAPON_NONE = 0,

	OF_WEAPON_BAT,
	OF_WEAPON_BAT_WOOD, // this is the sandman, only putting it here for refrence, remove in the future!
	OF_WEAPON_BOTTLE,
	OF_WEAPON_FIREAXE,
	OF_WEAPON_CLUB, // snipers kukri
	OF_WEAPON_CROWBAR,
	OF_WEAPON_KNIFE,
	OF_WEAPON_FISTS,
	OF_WEAPON_SHOVEL,
	OF_WEAPON_WRENCH,
	OF_WEAPON_BONESAW,
	OF_WEAPON_SHOTGUN, // maybe rename to OF_WEAPON_SHOTGUN_PRIMARY for compatibility? - cherry
	OF_WEAPON_SHOTGUN_SOLDIER,
	OF_WEAPON_SHOTGUN_HWG,
	OF_WEAPON_SHOTGUN_PYRO,
	OF_WEAPON_SCATTERGUN,
	OF_WEAPON_SNIPERRIFLE,
	OF_WEAPON_MINIGUN,
	OF_WEAPON_SMG,
	OF_WEAPON_SYRINGEGUN_MEDIC,
	OF_WEAPON_TRANQ,
	OF_WEAPON_ROCKETLAUNCHER,
	OF_WEAPON_GRENADELAUNCHER,
	OF_WEAPON_PIPEBOMBLAUNCHER,
	OF_WEAPON_FLAMETHROWER,

	// pretty sure these are just the cut grenades - cherry
	OF_WEAPON_GRENADE_NORMAL,
	OF_WEAPON_GRENADE_CONCUSSION,
	OF_WEAPON_GRENADE_NAIL,
	OF_WEAPON_GRENADE_MIRV,
	OF_WEAPON_GRENADE_MIRV_DEMOMAN,
	OF_WEAPON_GRENADE_NAPALM,
	OF_WEAPON_GRENADE_GAS,
	OF_WEAPON_GRENADE_EMP,
	OF_WEAPON_GRENADE_CALTROP,
	OF_WEAPON_GRENADE_PIPEBOMB,
	OF_WEAPON_GRENADE_SMOKE_BOMB,
	OF_WEAPON_GRENADE_HEAL,
	OF_WEAPON_GRENADE_STUNBALL,

	OF_WEAPON_GRENADE_JAR,
	OF_WEAPON_GRENADE_JAR_MILK,

	OF_WEAPON_PISTOL, // engineer pistol
	OF_WEAPON_PISTOL_SCOUT,
	OF_WEAPON_REVOLVER,
	OF_WEAPON_NAILGUN,
	OF_WEAPON_PDA,
	OF_WEAPON_PDA_ENGINEER_BUILD,
	OF_WEAPON_PDA_ENGINEER_DESTROY,
	OF_WEAPON_PDA_SPY,
	OF_WEAPON_BUILDER,
	OF_WEAPON_MEDIGUN,
	OF_WEAPON_GRENADE_MIRVBOMB,
	OF_WEAPON_FLAMETHROWER_ROCKET,
	OF_WEAPON_GRENADE_DEMOMAN,
	OF_WEAPON_SENTRY_BULLET,
	OF_WEAPON_SENTRY_ROCKET,
	OF_WEAPON_DISPENSER,
	OF_WEAPON_INVIS,

	// everything after the above weapon is econ in ghidra - cherry

    WEAPON_OFTODO,
    WEAPON_MAX,
} OFWeaponID;

const char *WeaponIDToAlias( int id );

#if defined( CLIENT_DLL )
    #define COFWeaponBase C_OFWeaponBase
#endif

// NOTE: Some methods in ghidra don't take `this` ptr, such
//  as CanFireCriticalShot
// Hypothesis: __thiscall calling convention places this in EAX,
//  and if you don't use it, you can simply ignore the value in
//  EAX and use the default calling conv.
// -Nopey


/*
 OFTODO: low priority: implement and make weaponbase inherit IHasGenericMeter (for shields and stuff, i think)
 IHasGenericMeter Notes
    ShouldUpdateMeter
    GetMeterMultiplier
    OnResourceMeterFilled
    GetChargeInterval
*/

class CTraceFilterTeam : public CTraceFilterSimple
{
public:
	DECLARE_CLASS(CTraceFilterTeam, CTraceFilterSimple);
	CTraceFilterTeam(const IHandleEntity *passentity, int collisionGroup, int filterteam)
		: CTraceFilterSimple(passentity, collisionGroup), m_iFilterTeam(filterteam)
	{
	}

	virtual bool ShouldHitEntity(IHandleEntity *pHandleEntity, int contentsMask)
	{
		CBaseEntity *pEntity = EntityFromEntityHandle(pHandleEntity);
		if (pEntity && pEntity->IsPlayer() && pEntity->GetTeamNumber() == m_iFilterTeam)
		{
			return false;
		}
		return true;
	}

	int m_iFilterTeam;
};

class COFPlayer;

//OFTODO: Mark many COFWeaponBase getters const
class COFWeaponBase: public CBaseCombatWeapon, IHasOwner /*, IHasGenericMeter */ 
{
    DECLARE_CLASS(COFWeaponBase, CBaseCombatWeapon);
    DECLARE_NETWORKCLASS();
    DECLARE_PREDICTABLE();

    #ifdef GAME_DLL
        DECLARE_DATADESC();
    #endif
    //NOTE: I'm basing this class interface entirely on the serverside vtable, and so..
    //OFTODO: Compare COFWeaponBase members to client.dylib CTFWeaponBase

    COFWeaponBase();
    #ifdef CLIENT_DLL
    ~COFWeaponBase();
    #endif
    // CBaseEntity::
    virtual const char *GetTracerType() override;

    // CBaseCombatWeapon::
    virtual void Spawn() override;
    virtual void Activate() override;

    // CBaseEntity::
    virtual void Precache() override;
    #ifdef GAME_DLL
    virtual void ChangeTeam(int iTeamNum) override;
	virtual void Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value ) override { return; };
	#endif
    // virtual void UpdateOnRemove() override; // econ, ignore it - cherry
	#ifdef GAME_DLL
    virtual CBaseEntity *Respawn () override;
	#endif
    virtual int GetDamageType() const;

    // virtual void ReapplyProvision();
    // virtual Activity TranslateViewmodelHandActivityInternal(Activity);

    // All predicted weapons need to implement and return true
	virtual bool        IsPredicted() const { return true; }
    virtual OFWeaponID  GetWeaponID() const { return WEAPON_NONE; }

    // CBaseCombatWeapon::
    virtual void Equip( CBaseCombatCharacter *pOwner ) override;
    virtual void Drop( const Vector &vecVelocity ) override;
    virtual bool VisibleInWeaponSelection() override;
    virtual bool HasAmmo() override;
    //virtual bool SendWeaponAnim( int iActivity ) override; // overrided just to inspect your weapon, ignored
    //virtual bool CanHolster() const override; // econ ignored
    virtual bool Deploy() override;
    virtual bool Holster( CBaseCombatWeapon *pSwitchingTo ) override;
    virtual void SetWeaponVisible( bool visible ) override;
    //virtual void OnActiveStateChanged( int iOldState ) override; // econ ignored
    virtual void Detach() override;
    virtual void ItemPostFrame() override;
	virtual void ReloadSinglyPostFrame();
	virtual bool Reload();
	virtual bool ReloadSingly();
	virtual void IncrementAmmo();
	virtual void SetReloadTimer( float flReloadTime );
    virtual void ItemBusyFrame() override;
    // virtual void ItemHolsterFrame() override; // ignore
    virtual void WeaponIdle() override;
    virtual bool CanPerformSecondaryAttack() const override;
    virtual void CheckReload() override;
    virtual void FinishReload() override;
    virtual void AbortReload() override;
    // virtual bool Reload() override;
    #ifdef CLIENT_DLL
    virtual bool AutoFiresFullClip() const override { return false; };
    #endif
    virtual void PrimaryAttack() override;
    virtual void SecondaryAttack() override;
    virtual const Vector &GetBulletSpread();
    virtual const char *GetViewModel() const;
    virtual const char *GetWorldModel() const override;
    // virtual int GetMaxClip1() const override; // ignore
    virtual int GetDefaultClip1() const override;
    //virtual bool ForceWeaponSwitch() const override; // ignore
    virtual const char *GetShootSound( int iIndex ) const override;
    virtual int Clip1() override;
    virtual int Clip2() override;
    //virtual Activity ActivityOverride( Activity baseAct, bool *pRequired ); // ignore, ghidra messed up
    virtual acttable_t *ActivityList( int &iActivityCount );
    #ifdef CLIENT_DLL
    virtual void FallInit() { /*Intentionally blank*/ };
    #endif
	virtual void Materialize();
    // virtual void CheckRespawn() override;
    virtual void Operator_HandleAnimEvent( animevent_t *pEvent, CBaseCombatCharacter *pOperator );
    virtual bool Ready() override;
    virtual bool Lower() override;
    
    // ghidra says it returns a float10.. magical.
    virtual float GetNextSecondaryAttackDelay();
    // probably returns an enum
	// nope! - cherry
	virtual int GetCustomDamageType() { return 0; };
	// energy weapons dont exist, so we can easily trim this
	virtual bool UsesPrimaryAmmo() { return true; };

    // again with float10, ghidra. I wonder what it means..
    // virtual float UberChargeAmmoPerShot();
    // some kind of pointer? maybe a reference.
    // virtual void *GetPlayerPoseParamList(int *);
    // virtual void *GetItemPoseParamList(int *);

    // IHasOwner::
    CBaseEntity *GetOwnerViaInterface () override;

    virtual void StartHolsterAnim();
	#ifdef CLIENT_DLL
    // virtual void UpdateHands(); // ignored! - cherry
	#endif
    // virtual bool OwnerCanTaunt();
    // virtual bool CanBeCritBoosted();
    // OFTODO: this is almost certainly not an int pointer.
    // virtual int *GetTauntItem();
    // warning: massive chonkey function
    // virtual void UpdateExtraWearables();
    // virtual void ExtraWearableEquipped(CTFWearable *);
    // virtual void ExtraWearableViewModelEquipped(CTFWearable *);
    // // awful name, just awful.
    virtual bool HideAttachmentsAndShowBodygroupsWhenPerformingWeaponIndependentTaunt() const;
    // virtual void RemoveExtraWearables();
    virtual void Misfire();
    virtual void CalcIsAttackCritical();
    virtual void FireFullClipAtOnce(){ return; };
    virtual bool CalcIsAttackCriticalHelper();
    //virtual bool CalcIsAttackCriticalHelperNoCrits();
    //virtual int GetPenetrateType();
    // some of these could be references. (Any that aren't null checked, make into refs.)
    virtual void GetProjectileFireSetup(COFPlayer *pPlayer, Vector param_2, Vector *param_3, QAngle *param_4, bool param_5, float param_6);
    // This return type is almost certainly incorrect.
    // virtual QAngle GetSpreadAngles(); NOTE: do pPlayer->EyeAngles() instead
    virtual bool IsFiring() const;
    virtual bool AreRandomCritsEnabled();
	virtual bool ShouldPlayClientReloadSound() { return false; }
    virtual bool DefaultReload( int iClipSize1, int iClipSize2, int iActivity );
    virtual bool IsReloading() const;
    virtual float GetReloadSpeedScale() const;
    virtual bool CheckReloadMisfire() const;
    virtual bool CanDrop() const;
    virtual bool AllowTaunts() const;
    // virtual float ApplyFireDelay( float param_1 ); ignore: mannpower
    // virtual int GetActivityWeaponRole() const; ignore: econ
    virtual int GetViewModelWeaponRole();
	virtual void AddAssociatedObject() { return; } // this may go unused?
	virtual void RemoveAssociatedObject() { return; } // this may go unused?
	// virtual void ApplyOnHitAttributes(CBaseEntity *param_1, COFPlayer *pPlayer, CTakeDamageInfo &info);
    // virtual void ApplyPostHitEffects(CTakeDamageInfo *, CTFPlayer *);
    // virtual void ApplyOnInjuredAttributes(CTFPlayer *,CTFPlayer *,CTakeDamageInfo *);
    // return types may be incorrect here
    // virtual bool DeflectProjectiles();
    // OFTODO: Look for other implementations of CTFWeaponBase::DeflectPlayer (pyro flamethrower, probs) and check this signature.
    // virtual bool DeflectPlayer(CTFPlayer *,CTFPlayer *,Vector *);
    // virtual bool DeflectEntity(CBaseEntity *,CTFPlayer *,Vector *);
    // virtual void PlayDeflectionSound(bool);
    // virtual float GetDeflectionRadius();
	virtual float GetJarateTime() { return 0.0f; };
    virtual bool CanAttack();
    virtual bool GetCanAttackFlags() const { return false; };
    virtual void WeaponReset();
    // virtual void WeaponRegenerate();
    virtual const char *GetMuzzleFlashEffectName_3rd();
    virtual const char *GetMuzzleFlashEffectName_1st();
    virtual const char *GetMuzzleFlashModel();
    virtual float GetMuzzleFlashModelLifetime();
    virtual const char *GetMuzzleFlashParticleEffect();
    // virtual const char *GetInventoryModel();
    virtual float GetSpeedMod();
	virtual bool CanFireCriticalShot() { return true; };
    // virtual bool CanFireRandomCriticalShot(CBaseEntity *);
    // Probably the MvM rottenburg cap stun :P
    virtual void OnControlStunned();
    virtual bool IsViewModelFlipped();
	virtual bool HideWhileStunned();
    virtual int GetMaxHealthMod();
    virtual float GetLastDeployTime();
    virtual bool IsEnergyWeapon() const { return false; };
    virtual bool IsBlastImpactWeapon() const { return false; };
	// OFSTATUS: COMPLETE
    virtual float Energy_GetShotCost(){ return 4.0f; };
	// OFSTATUS: COMPLETE
    virtual float Energy_GetRechargeCost(){ return 4.0f; };
    virtual Vector GetParticleColor(int iColor);
    virtual bool HasLastShotCritical() const { return false; };
    virtual bool UseServerRandomSeed() const { return true; };
    // int param is probs a bool or an enum
    // void OnBulletFire(int iPlayersHit); // ignored
    // virtual void OnPlayerKill(CTFPlayer *pSmellyUnfortunate, CTakeDamageInfo *);
    #ifdef GAME_DLL
    //virtual float GetLastHitTime(); // ignore
	virtual int GetDropSkinOverride() { return -1; };
	#endif
	virtual bool WeaponShouldBeLowered();
    virtual float GetInitialAfterburnDuration() { return 0.0f; };
    virtual float GetAfterburnRateOnHit() { return 0.0f; };
    // return value is probably the model's skin index
    // virtual uint GetSkin();
    virtual float GetEffectBarProgress();
    virtual bool CanPickupOtherWeapon() const { return true; };
    virtual bool ShouldRemoveInvisibilityOnPrimaryAttack() const { return true; };
    virtual int GetEffectBarAmmo();
    virtual float InternalGetEffectBarRechargeTime() { return 0.0f; };
    virtual bool CanInspect() { return true; } ;
    virtual void HookAttributes();
    #ifdef GAME_DLL
    virtual bool YouForgotToImplementOrDeclareServerClass() const { return 0; };
	virtual datamap_t *GetDataDescMap() const { return &m_DataMap; };
    #endif
    // NOTE: This MvM function literally just calls HookAttributes, unless ghidra is malfunctioning.
    // (oh, and it doesn't have any overrides)
    // or in other words, all my homies hate CTFWeaponBase::OnUpgraded
    // void OnUpgraded();
	
	COFPlayer	*GetOFPlayerOwner() const;
	
	const COFWeaponInfo	&GetOFWpnData( void ) const;
	
public:
	int m_iWeaponMode; // Used in stuff like airblast 'n similar
	CNetworkVar( int, m_iReloadStage );
private:
	CNetworkVar( bool, m_bAnimReload );
	CNetworkVar( bool, m_bInAttack );
	CNetworkVar( bool, m_bInAttack2 );
	CNetworkVar( float, m_flEnergy );
	CNetworkVar( int, m_iConsecutiveShots );
	CNetworkVar( int, m_iOldClip );
	CNetworkVar( float, m_flOldPrimaryAttack );
	float m_flLastDeployTime; // i'm not 100% if this is a cnetworkvar but it doesnt seem to be, do correct me if i'm wrong! - cherry
    bool m_bCanDropWeapon;
	char m_szTracerTypeName[128];
	COFWeaponInfo *m_pWeaponInfo;
	int field_0x6cc; // OFTODO: RENAME ME IN THE FUTURE!
	int m_iCritSeed;
	float m_flCritDuration;
	bool m_bLoweredWeapon;
	int m_iLastCritCheck;
	bool m_bAttackCritical;

	static acttable_t m_acttableSecondary[];
	static acttable_t m_acttableMelee[];
	static acttable_t m_acttablePrimary[];
	static acttable_t m_acttableBuilding[];
	static acttable_t m_acttablePDA[];
};
