// ========= Copyright Open Fortress Developers, CC-BY-NC-SA ============
// Purpose: Impl COFWeaponBase, the root of all OF weapons
// Author(s): Nopey, Fenteale, KaydemonLP
//
#include "cbase.h"
#ifdef CLIENT_DLL
#include "c_of_player.h"
#include "c_baseviewmodel.h"
#else
#include "of_player.h"
#include "baseviewmodel.h"
#endif
#include "of_weapon_base.h"
#include "in_buttons.h"
#include "of_shareddefs.h"
#include "activitylist.h"
#include "baseviewmodel_shared.h"

// ----------------------------------------------------------------------------- //
// Global functions.
// ----------------------------------------------------------------------------- //

//--------------------------------------------------------------------------------------------------------
static const char * s_WeaponAliasInfo[] = 
{
	// OFTODO: there might be some econ weapons here and cut weapons, they are only for refrence
	//			to make sure the other weapon ids are correct, remove them in the future!

	"none",								// WEAPON_NONE

	"tf_weapon_bat",					// OF_WEAPON_BAT
	"tf_weapon_bat_wood",				// OF_WEAPON_BAT_WOOD - this is the sandman, only putting it here for refrence, remove in the future! - cherry
	"tf_weapon_fireaxe",				// OF_WEAPON_FIREAXE
	"tf_weapon_club",					// OF_WEAPON_CLUB
	"tf_weapon_crowbar",				// OF_WEAPON_CROWBAR
	"tf_weapon_knife",					// OF_WEAPON_KNIFE
	"tf_weapon_fists",					// OF_WEAPON_FISTS
	"tf_weapon_shovel",					// OF_WEAPON_SHOVEL
	"tf_weapon_wrench",					// OF_WEAPON_WRENCH
	"tf_weapon_bonesaw",				// OF_WEAPON_BONESAW
	"tf_weapon_shotgun",				// OF_WEAPON_SHOTGUN - maybe rename to tf_weapon_shotgun_primary for compatibility? - cherry
	"tf_weapon_shotgun_soldier",		// OF_WEAPON_SHOTGUN_SOLDIER
	"tf_weapon_shotgun_hwg",			// OF_WEAPON_SHOTGUN_HWG
	"tf_weapon_shotgun_pyro",			// OF_WEAPON_SHOTGUN_PYRO
	"tf_weapon_scattergun",				// OF_WEAPON_SCATTERGUN
	"tf_weapon_sniperrifle",			// OF_WEAPON_SNIPERRIFLE
	"tf_weapon_minigun",				// OF_WEAPON_MINIGUN
	"tf_weapon_smg",					// OF_WEAPON_SMG
	"tf_weapon_syringegun_medic",		// OF_WEAPON_SYRINGEGUN_MEDIC
	"tf_weapon_tranq",					// OF_WEAPON_TRANQ
	"tf_weapon_rocketlauncher",			// OF_WEAPON_ROCKETLAUNCHER
	"tf_weapon_grenadelauncher",		// OF_WEAPON_GRENADELAUNCHER
	"tf_weapon_pipebomblauncher",		// OF_WEAPON_PIPEBOMBLAUNCHER
	"tf_weapon_flamethrower",			// OF_WEAPON_FLAMETHROWER

	// pretty sure these are just the cut grenades - cherry
	"tf_weapon_grenade_normal",			// OF_WEAPON_GRENADE_NORMAL
	"tf_weapon_grenade_concussion",		// OF_WEAPON_GRENADE_CONCUSSION
	"tf_weapon_grenade_nail",			// OF_WEAPON_GRENADE_NAIL
	"tf_weapon_grenade_mirv",			// OF_WEAPON_GRENADE_MIRV
	"tf_weapon_grenade_mirv_demoman",	// OF_WEAPON_GRENADE_MIRV_DEMOMAN
	"tf_weapon_grenade_napalm",			// OF_WEAPON_GRENADE_NAPALM
	"tf_weapon_grenade_gas",			// OF_WEAPON_GRENADE_GAS
	"tf_weapon_grenade_emp",			// OF_WEAPON_GRENADE_EMP
	"tf_weapon_grenade_caltrop",		// OF_WEAPON_GRENADE_CALTROP
	"tf_weapon_grenade_pipebomb",		// OF_WEAPON_GRENADE_PIPEBOMB
	"tf_weapon_grenade_smoke_bomb",		// OF_WEAPON_GRENADE_SMOKE_BOMB
	"tf_weapon_grenade_heal",			// OF_WEAPON_GRENADE_HEAL
	"tf_weapon_grenade_stunball",		// OF_WEAPON_GRENADE_STUNBALL

	// econ
	"tf_weapon_grenade_jar",			// OF_WEAPON_GRENADE_JAR
	"tf_weapon_grenade_jar_milk",		// OF_WEAPON_GRENADE_JAR_MILK

	"tf_weapon_pistol",					// OF_WEAPON_PISTOL
	"tf_weapon_pistol_scout",			// OF_WEAPON_PISTOL_SCOUT
	"tf_weapon_revolver",				// OF_WEAPON_REVOLVER
	"tf_weapon_nailgun",				// OF_WEAPON_NAILGUN
	"tf_weapon_pda",					// OF_WEAPON_PDA
	"tf_weapon_pda_engineer_build",		// OF_WEAPON_PDA_ENGINEER_BUILD
	"tf_weapon_pda_engineer_destroy",	// OF_WEAPON_PDA_ENGINEER_DESTROY
	"tf_weapon_pda_spy",				// OF_WEAPON_PDA_SPY
	"tf_weapon_builder",				// OF_WEAPON_BUILDER
	"tf_weapon_medigun",				// OF_WEAPON_MEDIGUN
	"tf_weapon_grenade_mirvbomb",		// OF_WEAPON_GRENADE_MIRVBOMB
	"tf_weapon_flamethrower_rocket",	// OF_WEAPON_FLAMETHROWER_ROCKET
	"tf_weapon_grenade_demoman",		// OF_WEAPON_GRENADE_DEMOMAN
	"tf_weapon_sentry_bullet",			// OF_WEAPON_SENTRY_BULLET
	"tf_weapon_sentry_rocket",			// OF_WEAPON_SENTRY_ROCKET
	"tf_weapon_dispenser",				// OF_WEAPON_DISPENSER
	"tf_weapon_invis",					// OF_WEAPON_INVIS

	NULL,								// WEAPON_OFTODO
	NULL,								// WEAPON_MAX
};

//--------------------------------------------------------------------------------------------------------
//
// Given an alias, return the associated weapon ID
//
int AliasToWeaponID( const char *alias )
{
	if (alias)
	{
		for( int i=0; s_WeaponAliasInfo[i] != NULL; ++i )
			if (!Q_stricmp( s_WeaponAliasInfo[i], alias ))
				return i;
	}

	return WEAPON_NONE;
}

//--------------------------------------------------------------------------------------------------------
//
// Given a weapon ID, return its alias
//
const char *WeaponIDToAlias( int id )
{
	if ( (id >= WEAPON_MAX) || (id < 0) )
		return NULL;

	return s_WeaponAliasInfo[id];
}

// ----------------------------------------------------------------------------- //
// COFWeaponBase tables.
// ----------------------------------------------------------------------------- //

IMPLEMENT_NETWORKCLASS_ALIASED( OFWeaponBase, DT_OFWeaponBase )

BEGIN_NETWORK_TABLE( COFWeaponBase, DT_OFWeaponBase )
#ifdef CLIENT_DLL
  
#else
/* SDK Garbo that might or might not be useful
    // world weapon models have no animations
      SendPropExclude( "DT_AnimTimeMustBeFirst", "m_flAnimTime" ),
    SendPropExclude( "DT_BaseAnimating", "m_nSequence" ),
*/
#endif
END_NETWORK_TABLE()

#ifdef CLIENT_DLL
BEGIN_PREDICTION_DATA( COFWeaponBase )
    DEFINE_PRED_FIELD( m_flTimeWeaponIdle, FIELD_FLOAT, FTYPEDESC_OVERRIDE | FTYPEDESC_NOERRORCHECK ),
END_PREDICTION_DATA()
#endif

// Do we need this? Weapon bases never get used on their own - Kay
LINK_ENTITY_TO_CLASS( tf_weapon_base, COFWeaponBase );


#ifdef GAME_DLL

    BEGIN_DATADESC( COFWeaponBase )

        // New weapon Think and Touch Functions go here..

    END_DATADESC()

#endif


// ----------------------------------------------------------------------------- //
// COFWeaponBase implementation. 
// ----------------------------------------------------------------------------- //

//OFSTATUS: INCOMPLETE, massive (~500 lines). (lots of fields, too)
COFWeaponBase::COFWeaponBase()
{
	m_iWeaponMode = OF_WEAPON_MODE_PRIMARY;
	m_flLastDeployTime = 0.0f;
	field_0x6cc = 0;
}

#ifdef CLIENT_DLL
//OFSTATUS: INCOMPLETE
COFWeaponBase::~COFWeaponBase()
{
    /*
    uint uVar1;
    C_BaseEntity *pCVar2;
    int *piVar3;
    undefined *puVar4;
    int iVar5;
    int iVar6;

    this->vtable = (vtableC_TFWeaponBase *)&PTR_ ~C_TFWeaponBase_00fbf338;
    this->field_0x4 = &PTR___ZThn4_N12C_BaseEntity17GetIClientUnknownEv_00fbfb64;
    this->field_0x8 = &PTR___ZThn8_N12C_BaseEntity17GetIClientUnknownEv_00fbfc20;
    this->field_0xc = &PTR___ZThn12_N12C_BaseEntity17GetIClientUnknownEv_00fbfc60;
    this->field_0x540 = &PTR___ZThn1344_N15C_BaseAnimating19OnModelLoadCompleteEPK7model_t_00fbfc7c;
    this->field_0x894 = &PTR___ZThn2196_N12C_EconEntity19GetAttributeManagerEv_00fbfc88;
    this->field_0xae0 = &PTR___ZThn2784_N14C_TFWeaponBase20GetOwnerViaInterfaceEv_00fbfca4;
    this->field_0xae4 = &PTR_ShouldUpdateMeter_00fbfcb0;
    this->field_0xae8 = &PTR___ZThn2792_N14C_TFWeaponBaseD1Ev_00fbfccc;
    puVar4 = PTR__g_pEntityList_00f8a1b4;
    iVar5 = *(int *)PTR__g_pEntityList_00f8a1b4;
    uVar1 = this->field_0xafc;
    if (((uVar1 != 0xffffffff) &&
         (iVar6 = (uVar1 & 0xfff) * 0x10, *(uint *)(iVar5 + 8 + iVar6) == uVar1 >> 0xc)) &&
        (pCVar2 = *(C_BaseEntity **)(iVar5 + 4 + iVar6), pCVar2 != (C_BaseEntity *)0x0))
    {
        C_BaseEntity::Remove(pCVar2);
        this->field_0xafc = 0xffffffff;
        iVar5 = *(int *)puVar4;
    }
    uVar1 = this->field_0xaf8;
    if (((uVar1 != 0xffffffff) &&
         (iVar6 = (uVar1 & 0xfff) * 0x10, *(uint *)(iVar5 + 8 + iVar6) == uVar1 >> 0xc)) &&
        (pCVar2 = *(C_BaseEntity **)(iVar5 + 4 + iVar6), pCVar2 != (C_BaseEntity *)0x0))
    {
        C_BaseEntity::Remove(pCVar2);
        this->field_0xaf8 = 0xffffffff;
    }
    this->field_0xae8 = &PTR_ ~CGameEventListener_011ff3b8;
    if (*(char *)&this->field_0xaec != '\0')
    {
        piVar3 = *(int **)PTR__gameeventmanager_00f8a094;
        if (piVar3 != (int *)0x0)
        {
            (**(code **)(*piVar3 + 0x18))(piVar3, &this->field_0xae8);
        }
        *(undefined *)&this->field_0xaec = 0;
    }
    C_BaseCombatWeapon::~C_BaseCombatWeapon((C_BaseCombatWeapon *)this);
    */

    /* Server Side Destructor
        this->~CBaseCombatWeapon();
    */
}
#endif

//OFSTATUS: COMPLETE
CBaseEntity *COFWeaponBase::GetOwnerViaInterface ()
{
    return CBaseCombatWeapon::GetOwner();
}

//OFSTATUS: COMPLETE
bool COFWeaponBase::VisibleInWeaponSelection()
{
    //NOTE: Trimmed code related to checking if the game is in training mode.
    return BaseClass::VisibleInWeaponSelection();
}

//OFSTATUS: COMPLETE
bool COFWeaponBase::HasAmmo()
{
    /* NOTE: Trimmed code
    if(IsEnergyWeapon())
        return true;
    else
    */
    return CBaseCombatWeapon::HasAmmo();
}

//OFSTATUS: INCOMPLETE
bool COFWeaponBase::CanHolster() const
{
    /*
    MysteryThing:  offset 0x769*4
    MysteryThing2: offset 0x768*4
    MysteryThing3: offset 0x42*4
    */

    // CBaseCombatCharacter* p_Character = CBaseCombatWeapon::GetOwner();
    /*
    //This comment block is not compiling, OFTODO: Fix this!
    if(p_Character && p_Character->IsPlayer())
        if(p_Character->GetActiveWeapon() != this || p_Character->MysteryThing <= gpGlobals + 0xc)
            // Only for Attributes stuff
            // if( CAttributeManager::AttribHookValue<int>(0, "honorbound", static_cast_pointer<CBaseEntity>(this), nullptr, true)
            if( p_Character->MysteryThing2 == 0 && p_Character->MysteryThing3 < 0x33)
                return false;
    */
    return true;
    
}

//OFSTATUS: COMPLETE
// bunch of econ and attrib stuff cut - cherry
bool COFWeaponBase::Deploy()
{
	#ifdef GAME_DLL

	COFPlayer *pPlayer = GetOFPlayerOwner();
	if (pPlayer)
	{
		pPlayer->StartHintTimer(field_0x6cc);
	}

	#endif

	m_iReloadStage.Set(OF_RELOAD_STAGE_NONE);

	float _flNextPrimaryAttack = m_flNextPrimaryAttack;
	float _flNextSecondaryAttack = m_flNextSecondaryAttack;

	bool bDeploy = CBaseCombatWeapon::Deploy();

	if (bDeploy)
	{
		m_flNextPrimaryAttack = max(_flNextPrimaryAttack, gpGlobals->curtime);
		m_flNextSecondaryAttack = max(_flNextSecondaryAttack, gpGlobals->curtime);
		m_flLastDeployTime = gpGlobals->curtime;
	}

	return bDeploy;
}

//OFSTATUS: INCOMPLETE
void COFWeaponBase::SetWeaponVisible(bool visible)
{
	BaseClass::SetWeaponVisible( visible );
	return;

    if(!visible)
    {
        AddEffects(EF_NODRAW);
        return;
    }
	else
	{
        RemoveEffects(EF_NODRAW);
        return;		
	}
    /*
    byte *pbVar1;
    ushort uVar2;
    vtableCTFWeaponBase *this_00; //pretty sure this is supposed to be another member
    ushort *puVar3;
    ushort *puVar4;
    ushort uVar5;
    uint uVar6;
    uint uVar7;
    vtableCTFWeaponBase *pvVar8; //pretty sure this is supposed to be another member
    uint local_24;
    

    pvVar8 = (vtableCTFWeaponBase *)((uint)this[0x37].vtable & 0xffffffdf);
    if (this[0x37].vtable != pvVar8)
    {
        if (*(char *)&this[0x17].vtable == '\0') {
            this_00 = this[8].vtable;
            if ((this_00 != (vtableCTFWeaponBase *)0x0) &&
            (((uint)this_00->CTFWeaponBase::~CTFWeaponBase & 0x100) == 0)) {
                this_00->CTFWeaponBase::~CTFWeaponBase =
                    (~CTFWeaponBase *)((uint)this_00->CTFWeaponBase::~CTFWeaponBase | 1);
                puVar4 = (ushort *)GetChangeAccessor((CBaseEdict *)this_00);
                puVar3 = *(ushort **)PTR__g_pSharedChangeInfo_00e31128;
                uVar2 = *puVar3;

                bool uCond1 = false;
                bool uCond2 = false;

                if (puVar4[1] == uVar2) {
                    uVar6 = (uint)*puVar4;
                    uVar2 = puVar3[uVar6 * 0x14 + 0x14];
                    local_24 = 0;
                    uVar5 = 0;
                    if (uVar2 != 0) {
                        uVar7 = 0;

                        uVar5 = uVar2;
                        local_24 = (uint)uVar2;
                        uCond1 = uVar2 == 0x13;
                        do {
                            if (puVar3[uVar6 * 0x14 + uVar7 + 1] == 0xdc) 
                            {
                                uCond2 = true;
                                break;
                            }
                            uVar7 = uVar7 + 1;
                        } while (uVar7 < (uint)uVar2);
                        
                        
                    }
                    if(uCond1 && !uCond2)
                        puVar4[1] = 0
                        pbVar1 = (byte *)((int)&this_00->CTFWeaponBase::~CTFWeaponBase + 1);
                        *pbVar1 = *pbVar1 | 1;
                    else if(!uCond2)
                    {
                        puVar3[uVar6 * 0x14 + 0x14] = uVar5 + 1;
                        puVar3[uVar6 * 0x14 + local_24 + 1] = 0xdc;
                    }
                }
                else {
                    uVar5 = puVar3[0x7d1];
                    if (uVar5 == 100) {
                        puVar4[1] = 0;
                        pbVar1 = (byte *)((int)&this_00->CTFWeaponBase::~CTFWeaponBase + 1);
                        *pbVar1 = *pbVar1 | 1;
                    }
                    else {
                        *puVar4 = uVar5;
                        puVar3[0x7d1] = uVar5 + 1;
                        puVar4[1] = uVar2;
                        puVar3[(uint)uVar5 * 0x14 + 1] = 0xdc;
                        puVar3[(uint)uVar5 * 0x14 + 0x14] = 1;
                    }
                }
            }
        }
        else {
            *(byte *)&this[0x18].vtable = *(byte *)&this[0x18].vtable | 1;
        }
        this[0x37].vtable = pvVar8;
    }

    pvVar8 = this[8].vtable;
    if (pvVar8 != (vtableCTFWeaponBase *)0x0) {
        *(byte *)&pvVar8->CTFWeaponBase::~CTFWeaponBase =
            *(byte *)&pvVar8->CTFWeaponBase::~CTFWeaponBase | 0x80;
    }
    DispatchUpdateTransmitState((CBaseEntity *)this);
    return;
    */
}

//OFSTATUS: COMPLETE
void COFWeaponBase::Detach() 
{
    return;
}

//OFSTATUS: INCOMPLETE
void COFWeaponBase::GetSpreadAngles()
{
/*
	char cVar1;
	int iVar2;
	int *piVar3;
	undefined8 *puVar4;
	int iVar5;
	int iVar6;
	int *piVar7;
	float fVar8;
	float fVar9;
	float fVar10;
	float fVar11;
	float fVar12;
	undefined8 *in_stack_00000004;
	int *in_stack_00000008;

	iVar2 = this->GetOwner();
	piVar7 = (int *)0x0;
	if ((iVar2 != 0) &&
		(piVar3 = (int *)__symbol_stub::___dynamic_cast
		(iVar2, PTR_typeinfo_00e34140, PTR_typeinfo_00e34088, 0),
		piVar3 != (int *)0x0)) {
		cVar1 = (**(code **)(*piVar3 + 0x144))(piVar3);
		piVar7 = (int *)0x0;
		if (cVar1 != '\0') {
			piVar7 = piVar3;
		}
	}
	puVar4 = (undefined8 *)(**(code **)(*piVar7 + 0x210))(piVar7);
	*(float *)(in_stack_00000004 + 1) = *(float *)(puVar4 + 1);
	*in_stack_00000004 = *puVar4;
	fVar12 = CAttributeManager::AttribHookValue<float>
		(0.0, "projectile_spread_angle", (CBaseEntity *)in_stack_00000008,
		(CUtlVector *)0x0, true);
	if (fVar12 != 0.0) {
		iVar2 = _rand();
		fVar8 = (float)iVar2 * fVar12 * 6.103702e-05 - fVar12;
		iVar2 = _rand();
		fVar12 = (float)iVar2 * fVar12 * 6.103702e-05 - fVar12;
		_rand();
		if ((((*(CTFGameRules **)PTR__g_pGameRules_00e340a8 == (CTFGameRules *)0x0) ||
			(cVar1 = CTFGameRules::GameModeUsesUpgrades(*(CTFGameRules **)PTR__g_pGameRules_00e340a8),
			cVar1 == '\0')) ||
			(iVar2 = CAttributeManager::AttribHookValue<int>
			(0, "can_overload", (CBaseEntity *)in_stack_00000008, (CUtlVector *)0x0, true
			), iVar2 == 0)) ||
			(((cVar1 = (**(code **)(*in_stack_00000008 + 0x46c))(in_stack_00000008), cVar1 == '\0' ||
			(iVar2 = (**(code **)(*in_stack_00000008 + 0x55c))(in_stack_00000008), iVar2 != 1)) ||
			((*(char *)((int)in_stack_00000008 + 0x5b6) != '\0' ||
			(fVar9 = *(float *)(*(int *)PTR__gpGlobals_00e34080 + 0xc) -
			(float)in_stack_00000008[0x1dd], 0.9 <= fVar9)))))) {
			fVar9 = 0.0;
		}
		else {
			fVar9 = (fVar9 + -0.4) * 2.0;
			if (fVar9 <= 0.0) {
				fVar9 = 0.0;
			}
			if (1.0 <= fVar9) {
				fVar9 = 1.0;
			}
			fVar10 = fVar9 * -5.0 + 6.0;
			iVar2 = _rand();
			fVar11 = fVar9 * -0.0003051851 + 0.0003662221;
			iVar5 = _rand();
			iVar6 = _rand();
			fVar9 = (float)iVar6 * fVar11 - fVar10;
			fVar8 = (fVar8 - fVar10) + (float)iVar2 * fVar11;
			fVar12 = (fVar12 - fVar10) + (float)iVar5 * fVar11;
		}
		*(float *)in_stack_00000004 = fVar8 + *(float *)in_stack_00000004;
		*(float *)((int)in_stack_00000004 + 4) = fVar12 + *(float *)((int)in_stack_00000004 + 4);
		*(float *)(in_stack_00000004 + 1) = fVar9 + *(float *)(in_stack_00000004 + 1);
	}
	*/
	return;
}

/* OFTODO: AAAAAAAA - Kay */
/* CTFWeaponBase::ItemPostFrame() */

//OFSTATUS: INCOMPLETE 90% done
// ----------------------------------------------------------------------------- //
// Purpoise: Runs every tick while the weapon isnt "Buisy" aka reloading or drawing
// Mostly handles barrages here ( Beggars/Old Panic attack )
// But also handles Singly reload frames
// ----------------------------------------------------------------------------- //
void COFWeaponBase::ItemPostFrame()
{
	COFPlayer *pOwner = GetOFPlayerOwner();
	if( !pOwner ) 
		return;

	int iMaxClip1 = GetMaxClip1();
	bool bClipIsNotFull = true;

	if( pOwner && UsesClipsForAmmo1() ) 
	{
		int iAmmoCount = pOwner->GetAmmoCount( m_iPrimaryAmmoType );
		if( iMaxClip1 - m_iClip1 < iAmmoCount ) 
		{
			iAmmoCount = iMaxClip1 - m_iClip1;
		}
		else 
		{
			iAmmoCount = pOwner->GetAmmoCount( m_iPrimaryAmmoType );
		}

		if( iAmmoCount != 0 ) 
		{
			bClipIsNotFull = false;
		}
	}
	else if( IsEnergyWeapon() ) 
	{
		float flMaxEnergy = 20.0f / Energy_GetShotCost(); // No attributes, so commented out AttribHook "mult_clipsize_upgrade",(20.0f / Energy_GetShotCost());
		bClipIsNotFull = (flMaxEnergy * Energy_GetShotCost()) > m_flEnergy;
	}

	if ( !AutoFiresFullClip() && pOwner->ShouldAutoReload() &&
      UsesClipsForAmmo1() && bClipIsNotFull
	  && !( (pOwner->m_nButtons & IN_ATTACK) || (pOwner->m_nButtons & IN_ATTACK2) ) )
	{
		pOwner->m_nButtons = pOwner->m_nButtons | IN_RELOAD;
	}

	if( m_bInAttack && !(pOwner->m_nButtons & IN_ATTACK) ) 
	{
		m_bInAttack = false;
	}

	if( m_bInAttack2 && !(pOwner->m_nButtons & IN_ATTACK2) ) 
	{
		m_bInAttack2 = false;
	}

/*
	We dont have any regen items since they're post release
	May add later if we decide to do so - Kay
	ApplyItemRegen();
	CheckEffectBarRegen();
  */
	// I think this is weapon being lowered/round lost? not 100% sure though so just commented out for now
	if ( true ) // !this[0x1b2].vtable
	{
		BaseClass::ItemPostFrame();

		if ( m_bReloadsSingly ) 
		{
			ReloadSinglyPostFrame();
		}
	
		bool bAutoFiresFullClpAtOnce = false; // No attributes, so commented out AttribHookValue<int>(0,"auto_fires_full_clip_all_at_once") > 0;
											  // Turn into function maybe
		// bAutoFiresFullClpAtOnce moved to front so it cancels faster
		if( bAutoFiresFullClpAtOnce && AutoFiresFullClip() && m_iClip1 == GetMaxClip1() )
		{
			FireFullClipAtOnce();
			return;
		}
	}
}

//OFSTATUS: COMPLETE
// ----------------------------------------------------------------------------- //
// Purpoise: Start the singly reload whenever needed
// ----------------------------------------------------------------------------- //
void COFWeaponBase::ReloadSinglyPostFrame()
{
	if( gpGlobals->curtime < m_flNextPrimaryAttack )
		return;

	if( IsEnergyWeapon() ) 
	{
		Reload();
		return;
	}
	
	// GetOwner() Null point check here maybe? Then again the function that calls it already does that - Kay
	if( !AutoFiresFullClip() && Clip1() == 0 && GetOwner()->GetAmmoCount( m_iPrimaryAmmoType ) > 0 ) 
		Reload();

	if( m_iReloadStage != OF_RELOAD_STAGE_NONE ) 
		Reload();
}

//OFSTATUS: INCOMPLETE 95% done
// ----------------------------------------------------------------------------- //
// Purpoise: Reload the weapon, or start a Singly Reload
// ----------------------------------------------------------------------------- //
bool COFWeaponBase::Reload()
{
	COFPlayer *pPlayer = GetOFPlayerOwner();
	if ( !pPlayer ) 
		return false;

	if( !pPlayer->IsPlayer() ) 
		return false;

/*
	// Uncomment when you figure out what this is - Kay
	*(undefined4 *)&this->field_0x788 = 0;
	*(undefined4 *)&this->field_0x78c = 0;
*/

	m_iConsecutiveShots = 0;
															// No attributes no problems
	float flMaxEnergy = 20.0f / Energy_GetShotCost();		// AttribHookValue<float>((float)(int)(20.00000000 / (float)Energy_GetShotCost()),"mult_clipsize_upgrade",(CBaseEntity *)this,(CUtlVector *)0x0,true); 
	
	// ( flMaxEnergy * Energy_GetShotCost() ) <= m_flEnergy seems to get called a lot, maybe turn into a function? - Kay
	if( !IsEnergyWeapon() || ( flMaxEnergy * Energy_GetShotCost() ) <= m_flEnergy ) 
	{
		if( m_iReloadStage == OF_RELOAD_STAGE_NONE ) 
		{
			if( GetOwner()->GetAmmoCount( m_iPrimaryAmmoType ) <= 0 ) 
				return false;

			bool bCanOverload = false; //AttribHookValue<int>(0,"can_overload",(CBaseEntity *)this,(CUtlVector *)0x0,true);
			if( !bCanOverload && GetMaxClip1() <= Clip1() ) 
				return false;
		}

		if( !m_bReloadsSingly ) 
		{
			DefaultReload( GetMaxClip1(), GetMaxClip2(), ACT_VM_RELOAD );
			return true;
		}
	}

	return ReloadSingly();
}

//OFSTATUS: INCOMPLETE 95% done
// ----------------------------------------------------------------------------- //
// Purpoise: Handle Singly reloads
// ----------------------------------------------------------------------------- //
bool COFWeaponBase::ReloadSingly()
{
	if( m_flNextPrimaryAttack > gpGlobals->curtime )
		return false;
	
	COFPlayer *pPlayer = GetOFPlayerOwner();
	if( !pPlayer ) 
		return false;

	if( !pPlayer->IsPlayer() ) 
		return false;

	bool bAutoFiresWhenFull = false; //AttribHookValue<int>(0,"auto_fires_when_full",(CBaseEntity *)this,(CUtlVector *)0x0,true);
	if( ( AutoFiresFullClip() && Clip1() > 0 && m_iReloadStage == OF_RELOAD_STAGE_NONE ) 
	|| ( bAutoFiresWhenFull && Clip1() == GetMaxClip1() || pPlayer->GetAmmoCount(m_iPrimaryAmmoType) <= 0 ) ) 
	{
		PrimaryAttack();
		//*(undefined *)&this->field_0x5b6 = 1; Does this mark the start of firing a barrage? - Kay
		return false;
	}

	switch( m_iReloadStage )
	{
		case OF_RELOAD_STAGE_NONE:
		{
			float flReloadTime;
			if ( !SendWeaponAnim(ACT_RELOAD_START) ) // Figure out what this enum is // Adendum: Probably just ACT_VM_RELOAD_START
			{
				flReloadTime = GetOFWpnData().m_WeaponModeInfo[m_iWeaponMode].m_flTimeReloadStart;
			}
			else
			{
				flReloadTime = SequenceDuration();
			}
	
			SetReloadTimer( flReloadTime );
			m_iReloadStage.Set( OF_RELOAD_STAGE_START );
			m_iOldClip = Clip1();
			return true;
		}
		break;
		case OF_RELOAD_STAGE_START:
		{
			if( m_flNextPrimaryAttack > gpGlobals->curtime )
				return false;

			PlayerAnimEvent_t iAnimEvent;
			if( Clip1() == m_iOldClip ) 
			{
				iAnimEvent = PLAYERANIMEVENT_RELOAD;
			}
			else 
			{
				iAnimEvent = PLAYERANIMEVENT_RELOAD_LOOP;
			}
//			This still crashes for some reason
//			pPlayer->DoAnimationEvent( iAnimEvent );

			m_bAnimReload = false;

			float flReloadTime;
			
			if ( !SendWeaponAnim( ACT_VM_RELOAD ) ) 
			{
				flReloadTime = GetOFWpnData().m_WeaponModeInfo[m_iWeaponMode].m_flTimeReload;
			}
			else 
			{
				if( GetWeaponID() == 0x17 ) 
				{
					flReloadTime = GetOFWpnData().m_WeaponModeInfo[m_iWeaponMode].m_flTimeReload;
				}
				else 
				{
					flReloadTime = SequenceDuration();
				}
			}

			SetReloadTimer( flReloadTime );
			WeaponSound( RELOAD );
			m_iReloadStage.Set( OF_RELOAD_STAGE_LOOP );
			return true;
		}
		break;
		case OF_RELOAD_STAGE_LOOP:
		{
			if( m_flNextPrimaryAttack > gpGlobals->curtime ) 
				return false;

			IncrementAmmo();

			if ( !IsEnergyWeapon() )
			{
				bool bCanOverload = false; //AttribHookValue<int>(0,"can_overload",(CBaseEntity *)this,(CUtlVector *)0x0,true);
				if( !bCanOverload && ( Clip1() == GetMaxClip1() || pPlayer->GetAmmoCount(m_iPrimaryAmmoType) <= 0 ) ) 
					m_iReloadStage.Set( OF_RELOAD_STAGE_END );
				else
					m_iReloadStage.Set( OF_RELOAD_STAGE_START );
			}
			else 
			{
				float flMaxEnergy = 20.0f / Energy_GetShotCost();
				if( flMaxEnergy * Energy_GetShotCost() <= m_flEnergy ) // Ditto from previous function, Maybe turn into a Func - Kay
				{
					m_iReloadStage.Set( OF_RELOAD_STAGE_END );
				}
				else 
				{
					m_iReloadStage.Set( OF_RELOAD_STAGE_START );
				}
			}
			return true;
		}
		break;
		default:
		case OF_RELOAD_STAGE_END:
		{
			SendWeaponAnim( ACT_RELOAD_FINISH );
			SetWeaponIdleTime( gpGlobals->curtime + SequenceDuration() );
//			This still crashes for some reason
//			pPlayer->DoAnimationEvent(PLAYERANIMEVENT_RELOAD_END);
			m_iReloadStage.Set( OF_RELOAD_STAGE_NONE );	
			return true;
		}
		break;
	}
}

//OFSTATUS: COMPLETE
// ----------------------------------------------------------------------------- //
// Purpoise: Increment the ammo if possible
// ----------------------------------------------------------------------------- //
void COFWeaponBase::IncrementAmmo()
{
	if( m_bAnimReload != false ) 
		return;
	
/*
	We dont have energy weapons so ignore for now - Kay
	if( IsEnergyWeapon() ) 
	{
		Energy_Recharge(this);
		return;
	}
*/
    COFPlayer *pPlayer = GetOFPlayerOwner();
	if( !pPlayer ) 
		return;
  
/*
	This also doesnt exist shut up - Kay
	if( CheckReloadMisfire() ) 
		return;
*/
	int iAmmoCount = pPlayer->GetAmmoCount(m_iPrimaryAmmoType);
	if( iAmmoCount <= 0 )
		return;

	if( m_iClip1 + 1 < GetMaxClip1() )
	{
		m_iClip1 += 1;
	}
	else 
	{
		m_iClip1 = GetMaxClip1();
	}

	pPlayer->RemoveAmmo( 1, m_iPrimaryAmmoType );
}

//OFSTATUS: INCOMPLETE 85% done
// ----------------------------------------------------------------------------- //
// Purpoise: Set the next reload and the viewmodel playback rate
// ----------------------------------------------------------------------------- //
void COFWeaponBase::SetReloadTimer( float flReloadTime )
{
	COFPlayer *pPlayer = GetOFPlayerOwner();
	if( !pPlayer ) 
		return;

	float flStartReloadTime = flReloadTime;

/*	Atribute stuff not needed - Kay
	AttribHookValue<float>(flReloadTime,"mult_reload_time",(CBaseEntity *)this,(CUtlVector *)0x0,true);
	AttribHookValue<float>(flReloadTime,"mult_reload_time_hidden",(CBaseEntity *)this,(CUtlVector *)0x0,true);
	AttribHookValue<float>(flReloadTime,"fast_reload",(CBaseEntity *)this,(CUtlVector *)0x0,true);
	AttribHookValue<float>(flReloadTime,"hwn_mult_reload_time",(CBaseEntity *)pPlayer,(CUtlVector *)0x0,true);
*/

/*
	We dont have m_Shared yet, nor Mann Power Powerups ( runes ) nor conds, so just skip for now - Kay
	
	float flReloadTimeMultiplier = 1.0f;
	
	if( pPlayer->m_Shared.GetCarryingRuneType() == 1 ) 
	{
		flReloadTimeMultiplier = 0.50000000;
	}
	else if( pPlayer->m_Shared.GetCarryingRuneType() == 9 || pPlayer->m_Shared.InCond( m_Shared,TF_COND_KING_BUFFED ) ) 
	{
		flReloadTimeMultiplier = 0.75000000;
	}

	flReloadTime = flReloadTime * flReloadTimeMultiplier;
*/
/*
	None of this makes sense, all the weird stuff at the start is probably something getting the healer?
	Anyways we dont need this since its all attribute stuff - Kay
	(*this->vtable->CTFWeaponBase::GetReloadSpeedScale)(this);
	auVar8 = ZEXT416((uint)(flReloadTime * (float)GetReloadSpeedScale())) &
           (undefined  [16])0xffffffffffffffff;
	if (*(int *)&pPlayer->field_0x1b40 == 1) 
	{
		flHealedReloadTime =
         AttribHookValue<float>
                   (SUB164(auVar8,0),"mult_reload_time_while_healed",(CBaseEntity *)pPlayer,
                    (CUtlVector *)0x0,true);
		auVar8 = ZEXT416((uint)flHealedReloadTime);
	}
	auVar8 = maxss(auVar8,0x3727c5ac);
	flFinalReloadTime = SUB164(auVar8,0);
	*/

	// Change to COFViewmodel whenver we have that
	CBaseViewModel *pViewmodel = pPlayer->GetViewModel(0);
	if( pViewmodel ) 
	{
		// This really looks like a calculation to get how quick the anim should be
		// So im gonna make a big assumption and say that all of the scribble below 
		// Is the network var for the anim speed updating
		float flAnimSpeed = flStartReloadTime / flReloadTime;
		pViewmodel->SetPlaybackRate( flAnimSpeed );
    }

	pViewmodel = pPlayer->GetViewModel(1);
	if( pViewmodel ) 
	{
		// This really looks like a calculation to get how quick the anim should be
		// So im gonna make a big assumption and say that all of the scribble below 
		// Is the network var for the anim speed updating
		float flAnimSpeed = flStartReloadTime / flReloadTime;
		pViewmodel->SetPlaybackRate( flAnimSpeed );
    }

	m_flOldPrimaryAttack = m_flNextPrimaryAttack;

	flReloadTime = flReloadTime + gpGlobals->curtime;

	pPlayer->m_flNextAttack = flReloadTime;

	flReloadTime = Max( (float)flReloadTime, (float)m_flOldPrimaryAttack );

	m_flNextPrimaryAttack = flReloadTime;

	SetWeaponIdleTime( flReloadTime );
}

//OFSTATUS: COMPLETE
bool COFWeaponBase::CanPerformSecondaryAttack() const
{
    //NOTE: Trimmed code for checking if demo shield
    
    return CBaseCombatWeapon::CanPerformSecondaryAttack();
}

//OFSTATUS: COMPLETE
bool COFWeaponBase::HideAttachmentsAndShowBodygroupsWhenPerformingWeaponIndependentTaunt() const
{
    return true;
}

//OFSTATUS: COMPLETE
void COFWeaponBase::Misfire()
{
    CalcIsAttackCritical();
}

//OFSTATUS: INCOMPLETE
void COFWeaponBase::CalcIsAttackCritical()
{
    //This is a very large and fat method.
}

//OFSTATUS: COMPLETE
bool COFWeaponBase::IsFiring() const {
    return false;
}

//OFSTATUS: INCOMPLETE
bool COFWeaponBase::AreRandomCritsEnabled()
{
    //wtf is DAT_0105baec ?
    //I will assume offset 0x30 is an integer pointer


    return false;

    //GetCurrentMatchGroup I think is a method of CTFGameRules.  Is this matchmaking stuff?
    //Also there is a class "ETFMatchGroup" that we dont have created yet
    /*
    int iVar1;
    bool bVar2;
    int local_c;

    if (*(int *)g_pGameRules == 0) {
        bVar2 = *(DAT_0105baec + 0x30) == 0;
    }
    else {
        if (*(char *)(*(int *)g_pGameRules + 0x961) != '\0') {
            bVar2 = true;  
        }
        else
        {
            local_c = GetCurrentMatchGroup();
            iVar1 = GetMatchGroupDescription((ETFMatchGroup *)&local_c);
            if (iVar1 == 0)
                bVar2 = *(DAT_0105baec + 0x30) == 0;
            else
                bVar2 = *(char *)(iVar1 + 0x43) == '\0';
        }
    }
    bVar2 = !bVar2;
    return (uint)bVar2;
    */
}

//OFSTATUS: COMPLETE
float COFWeaponBase::GetReloadSpeedScale() const {
    return 1.0f;
}

//OFSTATUS: COMPLETE
bool COFWeaponBase::CheckReloadMisfire() const {
    return false;
}

//OFSTATUS: COMPLETE
bool COFWeaponBase::CanDrop() const {
    return false;
}

//OFSTATUS: COMPLETE
bool COFWeaponBase::AllowTaunts() const {
    return true;
}

//OFSTATUS: INCOMPLETE, only temp for now
bool COFWeaponBase::CanAttack()
{
    /*
    undefined uVar1;
    int iVar2;
    C_TFPlayer *this_00;
    undefined3 extraout_var;

    iVar2 = this->GetOwner();
    if (iVar2 == 0)
    {
        uVar1 = 0;
    }
    else
    {
        this_00 = (C_TFPlayer *)
            __symbol_stub::___dynamic_cast(iVar2, PTR_typeinfo_00f8a488, PTR_typeinfo_00f8a508, 0);
        if (this_00 == (C_TFPlayer *)0x0)
        {
            uVar1 = 0;
        }
        else
        {
            uVar1 = this->GetCanAttackFlags();
            //uVar1 = C_TFPlayer::CanAttack(this_00, CONCAT31(extraout_var, uVar1));
        }
    }
    return uVar1;
    */
    return true;
}

//OFSTATUS: COMPLETE
void COFWeaponBase::StartHolsterAnim()
{
	Holster();
	return;
}

//OFSTATUS: COMPLETE
float COFWeaponBase::GetSpeedMod()
{
	return 1.0f;
}

//OFSTATUS: COMPLETE
// finished this as i had already done the code for it by accident, can be removed safely if for MvM - cherry
void COFWeaponBase::OnControlStunned()
{
	AbortReload();
	SetWeaponVisible(false);
}

//OFSTATUS: COMPLETE
bool COFWeaponBase::HideWhileStunned()
{
	return 1;
}

//OFSTATUS: INCOMPLETE
bool COFWeaponBase::IsViewModelFlipped()
{
	// Who uses flipped viewmodels anyways?

	return false;
	
	/*
	char cVar1;
	int iVar2;
	int *piVar3;
	bool bVar4;

	iVar2 = CBaseCombatWeapon::GetOwner((CBaseCombatWeapon *)this);
	if (iVar2 == 0)
	{
		bVar4 = false;
	}
	else
	{
		piVar3 = (int *)__symbol_stub::___dynamic_cast(iVar2, PTR_typeinfo_00e34140, PTR_typeinfo_00e34088, 0);
		if (piVar3 == (int *)0x0)
		{
			bVar4 = false;
		}
		else
		{
			cVar1 = (**(code **)(*piVar3 + 0x144))(piVar3);
			if (cVar1 == '\0')
			{
				bVar4 = false;
			}
			else
			{
				bVar4 = this[0x61c] != *(CTFWeaponBase *)(piVar3 + 0x7f3);
			}
		}
	}
	return bVar4;
	*/
}

//OFSTATUS: COMPLETE
int COFWeaponBase::GetMaxHealthMod()
{
	return 0;
}

//OFSTATUS: COMPLETE
float COFWeaponBase::GetLastDeployTime()
{
	return m_flLastDeployTime;
}

#ifdef GAME_DLL
//OFSTATUS: INCOMPLETE
// trimmed as that int is just used no where - cherry
void COFWeaponBase::OnBulletFire()
{
	/*
	this->field_0x788 = this->field_0x788 + 1;
	this->field_0x784 = gpGlobals->absoluteframetime;

	this->field_0x78c = this->field_0x78c + 1;
	return;
	*/
}

//OFSTATUS: INCOMPLETE
//float COFWeaponBase::GetLastHitTime()
//{
//	//return this->
//}
#endif

//-----------------------------------------------------------------------------
// Purpose: Get my data in the file weapon info array
//-----------------------------------------------------------------------------
const COFWeaponInfo &COFWeaponBase::GetOFWpnData( void ) const
{
	const FileWeaponInfo_t *pWeaponInfo = &GetWpnData();
	const COFWeaponInfo *pOFInfo;

	pOFInfo = static_cast< const COFWeaponInfo* >( pWeaponInfo );

	return *pOFInfo;
}

COFPlayer *COFWeaponBase::GetOFPlayerOwner() const
{
	return ToOFPlayer( GetOwner() );
}

// OFSTATUS: INCOMPLETE
// --------------------------------------
// Econ stuff removed + "complete" but we still gotta figure out where to define convar tf_useparticletracers - cherry
// --------------------------------------
const char *COFWeaponBase::GetTracerType()
{
	// OFTODO: this is a convar but we haven't defined it yet and im not sure where'd it gets defined,
	// and with testing in live tf2 it seems to be true by default, its also a hidden server convar too
	// so for the time being it'll be true - cherry

	//if (tf_useparticletracers.GetBool())
	if (true)
    {
		if (GetOFWpnData().m_szTracerParticle)
		{
			if (CBaseCombatWeapon::GetOwner())
			{
				if (GetOwner()->GetTeamNumber() == OF_TEAM_RED) Q_snprintf(m_szTracerTypeName, sizeof(m_szTracerTypeName), "%s_%s", GetOFWpnData().m_szTracerParticle, "red");
				if (GetOwner()->GetTeamNumber() == OF_TEAM_BLUE) Q_snprintf(m_szTracerTypeName, sizeof(m_szTracerTypeName), "%s_%s", GetOFWpnData().m_szTracerParticle, "blue");
			}
			else
			{
				Q_snprintf(m_szTracerTypeName, sizeof(m_szTracerTypeName), "%s_%s", GetOFWpnData().m_szTracerParticle, "blue");
			}
			return m_szTracerTypeName;
		}
    }

	if (GetWeaponID() == OF_WEAPON_MINIGUN)
    {
        return "BrightTracer";
    }

	// trimmed - cherry
    return NULL;
}

//OFSTATUS: COMPLETE
void COFWeaponBase::Spawn()
{
	CBaseCombatWeapon::Spawn();
	CBaseEntity::SetCollisionGroup(COLLISION_GROUP_WEAPON);
	FileWeaponInfo_t *pFileWeaponInfo = GetFileWeaponInfoFromHandle( LookupWeaponInfoSlot(CBaseEntity::GetClassname()) );
	if (pFileWeaponInfo)
	{
		COFWeaponInfo *pWeaponInfo = dynamic_cast<COFWeaponInfo*>(pFileWeaponInfo);
		m_pWeaponInfo = pWeaponInfo;
	}

	if (CBaseCombatWeapon::GetOwner())
	{
		COFWeaponBase::ChangeTeam( CBaseCombatWeapon::GetOwner()->CBaseEntity::GetTeamNumber() );
	}

	#ifdef GAME_DLL

	Vector org = GetAbsOrigin();
	CBaseEntity::SetAbsOrigin( Vector(org[0], org[1], org[2] + 5.0f) );

	#endif

	m_szTracerTypeName[0] = '\0';
}

// OFSTATUS: COMPLETE
// econ parts trimmed - cherry
void COFWeaponBase::Precache()
{
	BaseClass::Precache();

	if (COFWeaponBase::GetMuzzleFlashModel())
	{
		CBaseEntity::PrecacheModel(COFWeaponBase::GetMuzzleFlashModel());
	}

	//COFWeaponInfo weaponInfo = COFWeaponBase::GetOFWpnData();

	//if (*(char *)(iVar5 + 0x991) != '\0')
	if (COFWeaponBase::GetOFWpnData().m_szExplosionSound)
	{
		//CBaseEntity::PrecacheScriptSound((char *)(iVar5 + 0x991));
		CBaseEntity::PrecacheScriptSound(COFWeaponBase::GetOFWpnData().m_szExplosionSound);
	}

	//if (*(char *)(iVar5 + 0x911) != '\0')
	if (COFWeaponBase::GetOFWpnData().m_szBrassModel)
	{
		//CBaseEntity::PrecacheModel((char *)(iVar5 + 0x911));
		CBaseEntity::PrecacheModel(COFWeaponBase::GetOFWpnData().m_szBrassModel);
	}

	//uVar2 = (COFWeaponBase::GetMuzzleFlashParticleEffect);
	if (COFWeaponBase::GetMuzzleFlashParticleEffect())
	{
		//uVar2 = (COFWeaponBase::GetMuzzleFlashParticleEffect);
		PrecacheParticleSystem(COFWeaponBase::GetMuzzleFlashParticleEffect());
	}

	//if (*(char *)(iVar5 + 0xa11) != '\0')
	if (COFWeaponBase::GetOFWpnData().m_szExplosionEffect)
	{
		//PrecacheParticleSystem((char *)(iVar5 + 0xa11));
		PrecacheParticleSystem(COFWeaponBase::GetOFWpnData().m_szExplosionEffect);
	}

	//if (*(char *)(iVar5 + 0xa91) != '\0')
	if (COFWeaponBase::GetOFWpnData().m_szExplosionEffectPlayer)
	{
		//PrecacheParticleSystem((char *)(iVar5 + 0xa91));
		PrecacheParticleSystem(COFWeaponBase::GetOFWpnData().m_szExplosionEffectPlayer);
	}

	//if (*(char *)(iVar5 + 0xb11) != '\0')
	if (COFWeaponBase::GetOFWpnData().m_szExplosionEffectWater)
	{
		//PrecacheParticleSystem((char *)(iVar5 + 0xb11));
		PrecacheParticleSystem(COFWeaponBase::GetOFWpnData().m_szExplosionEffectWater);
	}

	if (COFWeaponBase::GetOFWpnData().m_szTracerParticle)
	{
		char szTracerparticle[128];
		char szTracerparticleCrit[128];

		V_snprintf(szTracerparticle, sizeof(szTracerparticle), "%s_red", COFWeaponBase::GetOFWpnData().m_szTracerParticle);
		V_snprintf(szTracerparticleCrit, sizeof(szTracerparticleCrit), "%s_red_crit", COFWeaponBase::GetOFWpnData().m_szTracerParticle);
		PrecacheParticleSystem(szTracerparticle);
		PrecacheParticleSystem(szTracerparticleCrit);
		V_snprintf(szTracerparticle, sizeof(szTracerparticle), "%s_blue", COFWeaponBase::GetOFWpnData().m_szTracerParticle);
		V_snprintf(szTracerparticleCrit, sizeof(szTracerparticleCrit), "%s_blue_crit", COFWeaponBase::GetOFWpnData().m_szTracerParticle);
		PrecacheParticleSystem(szTracerparticle);
		PrecacheParticleSystem(szTracerparticleCrit);
	}
}

//OFSTATUS: COMPLETE
void COFWeaponBase::Activate()
{
    this->Activate();
    this->GiveDefaultAmmo();
}

//OFSTATUS: COMPLETE
// huge econ trim - cherry
void COFWeaponBase::Equip( CBaseCombatCharacter *pOwner )
{
	CBaseCombatWeapon::SetOwner(pOwner);
	CBaseEntity::SetOwnerEntity(pOwner);
	CBaseCombatWeapon::Equip(pOwner);
}

//OFSTATUS: COMPLETE
// trimmed - cherry
void COFWeaponBase::Drop( const Vector &vecVelocity )
{
	#ifdef GAME_DLL
	if (field_0x6cc != 0)
    {
        if ( CBaseCombatWeapon::GetOwner() )
        {
			COFPlayer *pOFPlayer = GetOFPlayerOwner();
			if (pOFPlayer)
            {
				pOFPlayer->StopHintTimer(field_0x6cc);
            }
        }
    }
	#endif

    CBaseCombatWeapon::Drop(vecVelocity);

	#ifdef GAME_DLL
	UTIL_Remove(this);
	#endif
}

#ifdef GAME_DLL
//OFSTATUS: COMPLETE
void COFWeaponBase::ChangeTeam(int iTeamNum)
{
    // Trimmed the fat :3
    CBaseEntity::ChangeTeam(iTeamNum);
}
#endif

//OFSTATUS: INCOMPLETE
void COFWeaponBase::WeaponReset()
{
    /*
    byte bVar1;
    ushort uVar2;
    ushort uVar3;
    uint *this_00;
    ushort *puVar4;
    ushort *puVar5;
    uint uVar6;
    uint uVar7;
    uint uVar8;
    float10 extraout_ST0;
    float10 extraout_ST0_00;
    float fVar9;
    uint local_34;
    float local_18;
    int local_14;

    local_14 = 0;
    CNetworkVarBase<int, CTFWeaponBase::NetworkVar_m_iReloadMode>::Set((CNetworkVarBase<int, CTFWeaponBase::NetworkVar_m_iReloadMode> *)&this->field_0x6ac,
                                                                       &local_14);
    bVar1 = *(byte *)&this->field_0x76c;
    if (*(char *)&this->field_0x5c != '\0')
    {
        *(byte *)&this->field_0x60 = *(byte *)&this->field_0x60 | 1;
        goto LAB_00372efa;
    }
    this_00 = (uint *)this->field_0x20;
    if ((this_00 == (uint *)0x0) || ((*this_00 & 0x100) != 0))
        goto LAB_00372efa;
    *this_00 = *this_00 | 1;
    puVar5 = (ushort *)CBaseEdict::GetChangeAccessor((CBaseEdict *)this_00);
    puVar4 = *(ushort **)PTR__g_pSharedChangeInfo_00e34128;
    uVar2 = *puVar4;
    if (puVar5[1] == uVar2)
    {
        uVar6 = (uint)*puVar5;
        uVar8 = (uint)puVar4[uVar6 * 0x14 + 0x14];
        local_34 = 0;
        if (uVar8 != 0)
        {
            uVar7 = 0;
            do
            {
                if (puVar4[uVar6 * 0x14 + uVar7 + 1] == 0x76c)
                    goto LAB_00372efa;
                uVar7 = uVar7 + 1;
            } while (uVar7 < uVar8);
            local_34 = uVar8;
            if (uVar8 == 0x13)
            {
                puVar5[1] = 0;
                goto LAB_00372e9a;
            }
        }
        puVar4[uVar6 * 0x14 + 0x14] = (short)local_34 + 1;
        puVar4[uVar6 * 0x14 + local_34 + 1] = 0x76c;
    }
    else
    {
        uVar3 = puVar4[0x7d1];
        uVar6 = (uint)uVar3;
        if (uVar6 != 100)
        {
            *puVar5 = uVar3;
            puVar4[0x7d1] = uVar3 + 1;
            puVar5[1] = uVar2;
            puVar4[uVar6 * 0x14 + 1] = 0x76c;
            puVar4[uVar6 * 0x14 + 0x14] = 1;
            goto LAB_00372efa;
        }
        puVar5[1] = 0;
    LAB_00372e9a:
        *(CBaseEdict *)((int)this_00 + 1) = (CBaseEdict)((byte) * (CBaseEdict *)((int)this_00 + 1) | 1);
    }
LAB_00372efa:
    *(byte *)&this->field_0x76c = bVar1 ^ 1;
    (*this->vtable->CTFWeaponBase::Energy_GetShotCost)(this);
    fVar9 = CAttributeManager::AttribHookValue<float>((float)(int)(20.0 / (float)extraout_ST0), "mult_clipsize_upgrade",
                                                      (CBaseEntity *)this, (CUtlVector *)0x0, true);
    (*this->vtable->CTFWeaponBase::Energy_GetShotCost)(this);
    local_18 = (float)(int)fVar9 * (float)extraout_ST0_00;
    CNetworkVarBase<float, CTFWeaponBase::NetworkVar_m_flEnergy>::operator=((CNetworkVarBase<float, CTFWeaponBase::NetworkVar_m_flEnergy> *)&this->field_0x77c,
                                                                            &local_18);
    */

    /* Ghidra fucked up lole (Client)
    //WARNING: Could not recover jumptable at 0x00369a7f. Too many branches
    // WARNING: Treating indirect jump as call
    (*this->vtable->C_TFWeaponBase::UpdateVisibility)();
    */
}

#ifdef GAME_DLL
//OFSTATUS: COMPLETE
CBaseEntity *COFWeaponBase::Respawn()
{
	const char *pWeaponName = WeaponIDToAlias( GetWeaponID() );
	if (!pWeaponName)
	{
		pWeaponName = "";
	}

	CBaseEntity *pWeapon = CBaseEntity::Create(pWeaponName, g_pGameRules->VecWeaponRespawnSpot(this), CBaseEntity::GetAbsAngles(), CBaseCombatWeapon::GetOwner());

	if (pWeapon)
	{
		pWeapon->AddEffects(EF_NODRAW);
		pWeapon->SetThink(&CBaseCombatWeapon::AttemptToMaterialize);
		UTIL_DropToFloor(this, MASK_SOLID, NULL);
		pWeapon->SetNextThink(gpGlobals->absoluteframetime + g_pGameRules->FlWeaponRespawnTime(this), NULL);
	}
	else
	{
		// cutted out GetAbsAngles/puVar6, doesnt really make sense to have it there - cherry
		Msg("Respawn failed to create %s!\n", WeaponIDToAlias(GetWeaponID()));
	}
	return pWeapon;
}
#endif

//OFSTATUS: COMPLETE
int COFWeaponBase::GetDamageType() const
{
	return g_aWeaponDamageTypes[ GetWeaponID() ];
}

//OFSTATUS: COMPLETE
const char *COFWeaponBase::GetMuzzleFlashEffectName_3rd()
{
	return NULL;
}

//OFSTATUS: COMPLETE
const char *COFWeaponBase::GetMuzzleFlashEffectName_1st()
{
	return NULL;
}

//OFSTATUS: COMPLETE
const char *COFWeaponBase::GetMuzzleFlashModel()
{
	const char *pMuzzleFlashModel = GetOFWpnData().m_szMuzzleFlashModel;

	// we'll use V_ from now on as the Q_ are quake leftovers (thank you int for pointing this out) - cherry
	if (0 < V_strlen(pMuzzleFlashModel))
	{
		return pMuzzleFlashModel;
	}
	return NULL;
}

//OFSTATUS: COMPLETE
float COFWeaponBase::GetMuzzleFlashModelLifetime()
{
	return GetOFWpnData().m_flMuzzleFlashModelDuration;
}

//OFSTATUS: COMPLETE
// econ parts cut - cherry
const char *COFWeaponBase::GetMuzzleFlashParticleEffect()
{
	const char *pMuzzleFlashParticleEffect = GetOFWpnData().m_szMuzzleFlashParticle;

	if (0 < V_strlen(pMuzzleFlashParticleEffect))
	{
		return pMuzzleFlashParticleEffect;
	}
	return NULL;
}

//OFSTATUS: INCOMPLETE
Vector COFWeaponBase::GetParticleColor(int iColor)
{
    /*
    int *this;
    char cVar1;
    int iVar2;
    uint uVar3;
    int iVar4;
    float fVar5;
    float fVar6;
    int *in_stack_00000008;
    int in_stack_0000000c;

    uVar3 = in_stack_00000008[0x7f];
    if (uVar3 != 0xffffffff)
    {
        iVar4 = (uVar3 & 0xfff) * 0x10;
        if ((((*(uint *)(*(int *)PTR__g_pEntityList_00e3407c + 8 + iVar4) == uVar3 >> 0xc) &&
              (this = *(int **)(*(int *)PTR__g_pEntityList_00e3407c + 4 + iVar4), this != (int *)0x0)) &&
             (cVar1 = (**(code **)(*this + 0x144))(this), cVar1 != '\0')) &&
            (iVar4 = (**(code **)(*in_stack_00000008 + 0x37c))(), *(char *)(iVar4 + 0xb0) != '\0'))
        {
            iVar2 = CBaseEntity::GetTeamNumber((CBaseEntity *)this);
            uVar3 = CEconItemView::GetModifiedRGBValue((CEconItemView *)(iVar4 + 0x5c), iVar2 == 3);
            if (0 < (int)uVar3)
            {
                fVar6 = *(float *)(&DAT_00c01e10 + (uint)(in_stack_0000000c == 2) * 4);
                fVar5 = (float)(uVar3 >> 0x10 & 0xff) * fVar6;
                if (fVar5 <= 0.0)
                {
                    fVar5 = 0.0;
                }
                if (1.0 <= fVar5)
                {
                    fVar5 = 1.0;
                }
                *(float *)iColor = fVar5;
                fVar5 = (float)(uVar3 >> 8 & 0xff) * fVar6;
                if (fVar5 <= 0.0)
                {
                    fVar5 = 0.0;
                }
                if (1.0 <= fVar5)
                {
                    fVar5 = 1.0;
                }
                *(float *)(iColor + 4) = fVar5;
                fVar6 = (float)(uVar3 & 0xff) * fVar6;
                if (fVar6 <= 0.0)
                {
                    fVar6 = 0.0;
                }
                if (1.0 <= fVar6)
                {
                    fVar6 = 1.0;
                }
                *(float *)(iColor + 8) = fVar6;
                return;
            }
            iVar4 = CBaseEntity::GetTeamNumber((CBaseEntity *)this);
            if (in_stack_0000000c == 1)
            {
                if (iVar4 == 2)
                {
                    *(undefined4 *)iColor = 0x3f3851ec;
                    *(undefined4 *)(iColor + 4) = 0x3e6147ae;
                    *(undefined4 *)(iColor + 8) = 0x3e6b851f;
                    return;
                }
                *(undefined4 *)iColor = 0x3eb0a3d7;
                *(undefined4 *)(iColor + 4) = 0x3f051eb8;
                *(undefined4 *)(iColor + 8) = 0x3f228f5c;
                return;
            }
            if (iVar4 == 2)
            {
                *(undefined4 *)iColor = 0x3f000000;
                *(undefined4 *)(iColor + 4) = 0x3e3851ec;
                *(undefined4 *)(iColor + 8) = 0x3e000000;
                return;
            }
            *(undefined4 *)iColor = 0x3e147ae1;
            *(undefined4 *)(iColor + 4) = 0x3eda9fbe;
            *(undefined4 *)(iColor + 8) = 0x3f0ccccd;
            return;
        }
    }
    *(undefined4 *)iColor = 0;
    *(undefined4 *)(iColor + 4) = 0;
    *(undefined4 *)(iColor + 8) = 0;
    */

	return Vector(0, 0, 0); // TEMPORARY - cherry
}

//OFSTATUS: COMPLETE(?)
void COFWeaponBase::HookAttributes()
{
    // Intentionally Blank
}
