// ========= Copyright Open Fortress Developers, CC-BY-NC-SA ============
// Purpose: Impl COFWeaponBase, the root of all OF weapons
// Author(s): Nopey, Fenteale, KaydemonLP, Cherry!
//
#include "cbase.h"
#include "eventlist.h"
#include "of_shareddefs.h"
#ifdef CLIENT_DLL
#include "c_of_player.h"
#include "c_baseviewmodel.h"
#include "of_clientmode.h"
#else
#include "of_player.h"
#include "baseviewmodel.h"
#endif
#include "of_gamerules.h"
#include "of_weapon_base.h"
#include "in_buttons.h"

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
	"tf_weapon_bottle",					// OF_WEAPON_BOTTLE
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
	RecvPropBool(RECVINFO(m_bLoweredWeapon)),
	RecvPropInt(RECVINFO(m_iReloadStage)),
	RecvPropBool(RECVINFO(m_bAnimReload)),
	RecvPropFloat(RECVINFO(m_flOldPrimaryAttack))
#else
	SendPropBool(SENDINFO(m_bLoweredWeapon)),
	SendPropInt(SENDINFO(m_iReloadStage)),
	SendPropBool(SENDINFO(m_bAnimReload)),
	SendPropFloat(SENDINFO(m_flOldPrimaryAttack))
#endif
END_NETWORK_TABLE()

#ifdef CLIENT_DLL
BEGIN_PREDICTION_DATA( COFWeaponBase )
	DEFINE_PRED_FIELD( m_bLoweredWeapon, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_iReloadStage, FIELD_INTEGER, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_bAnimReload, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_flOldPrimaryAttack, FIELD_FLOAT, FTYPEDESC_INSENDTABLE ),
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

#ifdef CLIENT_DLL

extern ConVar cl_flipviewmodels;

#endif
ConVar tf_weapon_criticals( "tf_weapon_criticals", "1", FCVAR_NOTIFY | FCVAR_REPLICATED, "");

// ----------------------------------------------------------------------------- //
// COFWeaponBase implementation. 
// ----------------------------------------------------------------------------- //

//OFSTATUS: COMPLETE
COFWeaponBase::COFWeaponBase()
{
	SetPredictionEligible(true);
	AddSolidFlags(FSOLID_TRIGGER);

	m_bLoweredWeapon = false;
	m_iWeaponMode = OF_WEAPON_MODE_PRIMARY;
	m_flLastDeployTime = 0.0f;
	field_0x6cc = 0;
	m_iCritSeed = -1;
	m_flCritDuration = 0;
	m_iLastCritCheck = 0;
	m_bAttackCritical = false;
	m_bWeaponReset = false;
}

//OFSTATUS: COMPLETE
CBaseEntity *COFWeaponBase::GetOwnerViaInterface()
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

//OFSTATUS: COMPLETE
// cut econ related things - cherry
bool COFWeaponBase::Holster(CBaseCombatWeapon *pSwitchingTo)
{
	#ifdef GAME_DLL
	COFPlayer *pPlayer = GetOFPlayerOwner();
	if (field_0x6cc)
	{
		pPlayer->StopHintTimer(field_0x6cc);
	}
	#endif

	m_iReloadStage = OF_RELOAD_STAGE_NONE;
	m_iConsecutiveShots = 0;

	return CBaseCombatWeapon::Holster(pSwitchingTo);
}

//OFSTATUS: COMPLETE
// im pretty sure that middle part in the server.dylib is just ghidra dying, as compared to the client its a LOT more short
// and makes more sense since we're just changing the visibility of a weapon - cherry
void COFWeaponBase::SetWeaponVisible(bool visible)
{
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

	#ifdef CLIENT_DLL

	UpdateVisibility();

	CBaseAnimating::PreDataUpdate(DATA_UPDATE_DATATABLE_CHANGED);

	#endif
}

//OFSTATUS: COMPLETE
void COFWeaponBase::Detach() 
{
    return;
}

/* OFTODO: AAAAAAAA - Kay */
/* CTFWeaponBase::ItemPostFrame() */

//OFSTATUS: COMPLETE
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
	// I think this is weapon being lowered/round lost? not 100% sure though so just commented out for now - Kay
	// defintly looks like weapon being lowered - cherry
	if (m_bLoweredWeapon == false) // !this[0x1b2].vtable
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

//OFSTATUS: COMPLETE
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
		m_bFiringWholeClip = true;
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
			pPlayer->DoAnimationEvent( iAnimEvent );

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

		#ifdef CLIENT_DLL
			if (ShouldPlayClientReloadSound())
			{
				WeaponSound(RELOAD);
			}
		#else
			WeaponSound(RELOAD);
		#endif

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
			pPlayer->DoAnimationEvent(PLAYERANIMEVENT_RELOAD_END);
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
void COFWeaponBase::ItemBusyFrame()
{
	CBaseCombatWeapon::ItemBusyFrame();

	COFPlayer *pPlayer = GetOFPlayerOwner();

	if (pPlayer)
	{
		if (!(pPlayer->m_nButtons & IN_ATTACK2) && m_bInAttack2 == true)
		{
			m_bInAttack2 = false;
		}
		else 
		{
			if (!m_bInAttack2)
			{
				// OFTODO:
				//COFPlayer::DoClassSpecialSkill();
				m_bInAttack2 = true;
			}
		}
		if ((pPlayer->m_nButtons & IN_ATTACK))
		{
			if (0 < Clip1())
			{
				bool allowAbortReload = false;
				if (!m_bReloadsSingly)
				{
					if (m_bInReload && gpGlobals->curtime >= m_flOldPrimaryAttack)
					{
						allowAbortReload = true;
					}
				}
				else
				{
					if (m_iReloadStage != OF_RELOAD_STAGE_NONE)
					{
						m_iReloadStage.Set(OF_RELOAD_STAGE_NONE);
						allowAbortReload = true;
					}
				}

				if (allowAbortReload)
				{
					AbortReload();
					m_bInReload = false;
					pPlayer->m_flNextAttack = gpGlobals->curtime;
					m_flNextPrimaryAttack = Max(gpGlobals->curtime, (float)m_flOldPrimaryAttack);
					CBaseCombatWeapon::SetWeaponIdleTime(gpGlobals->curtime + GetOFWpnData().m_WeaponModeInfo[m_iWeaponMode].m_flTimeIdle);
				}
			}
		}
		//ApplyItemRegen(this);
		//CheckEffectBarRegen(this);
	}
}

//OFSTATUS: COMPLETE
bool COFWeaponBase::WeaponShouldBeLowered()
{
	// Can't be in the middle of another animation
	if (GetIdealActivity() != ACT_VM_IDLE_LOWERED && GetIdealActivity() != ACT_VM_IDLE &&
		GetIdealActivity() != ACT_VM_IDLE_TO_LOWERED && GetIdealActivity() != ACT_VM_LOWERED_TO_IDLE)
	{
		return false;
	}

	if (m_bLoweredWeapon) return true;

	return false;
}

//OFSTATUS: COMPLETE
void COFWeaponBase::WeaponIdle()
{
	if (WeaponShouldBeLowered())
	{
		if (((GetActivity() != ACT_VM_IDLE_TO_LOWERED) && (GetActivity() != ACT_TRANSITION)) || CBaseCombatWeapon::HasWeaponIdleTimeElapsed())
		{
			COFWeaponBase::SendWeaponAnim(ACT_VM_IDLE_LOWERED);
			return;
		}
	}

	if (GetActivity() == ACT_VM_IDLE_LOWERED)
	{
		COFWeaponBase::SendWeaponAnim(ACT_VM_IDLE);
	}
	else
	{
		if (m_bReloadsSingly == false || m_iReloadStage == OF_RELOAD_STAGE_NONE && CBaseCombatWeapon::HasWeaponIdleTimeElapsed() )
		{
			COFWeaponBase::SendWeaponAnim(ACT_VM_IDLE);
			SetWeaponIdleTime(gpGlobals->curtime + CBaseAnimating::SequenceDuration());
		}
	}
}

//OFSTATUS: COMPLETE
void COFWeaponBase::AbortReload()
{
	CBaseCombatWeapon::AbortReload();
	m_iReloadStage = OF_RELOAD_STAGE_NONE;
	if (-1 < CBaseAnimating::FindBodygroupByName("reload"))
	{
		CBaseAnimating::SetBodygroup(CBaseAnimating::FindBodygroupByName("reload"), 0);
	}
}

//OFSTATUS: COMPLETE
// cut the bottom part as i think its just for marking people for death in-game, so the fan o'war - cherry
void COFWeaponBase::PrimaryAttack()
{
	m_iWeaponMode = OF_WEAPON_MODE_PRIMARY;
	if (COFWeaponBase::CanAttack())
	{
		CBaseCombatWeapon::PrimaryAttack();
		if (m_bReloadsSingly)
		{
			m_iReloadStage.Set(OF_RELOAD_STAGE_NONE);
		}
	}
}

//OFSTATUS: COMPLETE
void COFWeaponBase::SecondaryAttack()
{
	m_iWeaponMode = OF_WEAPON_MODE_SECONDARY;
}

//OFSTATUS: COMPLETE
// im hoping the GetBulletSpread in basecombatweapon is the same here - cherry
const Vector &COFWeaponBase::GetBulletSpread()
{
	static Vector cone = VECTOR_CONE_15DEGREES;
	return cone;
}

//OFSTATUS: COMPLETE
// trimmed, as all it does is some econ stuff and something about the gunslinger
// which we dont care about - cherry
const char *COFWeaponBase::GetViewModel() const
{
	COFPlayer *pPlayer = GetOFPlayerOwner();
	if (pPlayer)
	{
		return GetOFWpnData().szViewModel;
	}

	return CBaseCombatWeapon::GetViewModel();
}

//OFSTATUS: COMPLETE
// once more, econ related, trimmed
const char *COFWeaponBase::GetWorldModel() const
{
	return CBaseCombatWeapon::GetWorldModel();
}

//OFSTATUS: COMPLETE
int COFWeaponBase::GetDefaultClip1() const
{
	return GetMaxClip1();
}

//OFSTATUS: COMPLETE
// trimmed: right as i seen the IsMiniBoss i knew it was just for those giant heavys in mvm with the diffrent minigun sound - cherry
const char *COFWeaponBase::GetShootSound(int iIndex) const
{
	return CBaseCombatWeapon::GetShootSound(iIndex);
}

//OFSTATUS: COMPLETE
int COFWeaponBase::Clip1()
{
	return m_iClip1;
}

//OFSTATUS: COMPLETE
int COFWeaponBase::Clip2()
{
	return m_iClip2;
}

// these were found by just converting the hex values to decimals, yes maunually, it was extremely boring - cherry
// new note: some of the values arent the same with our activitylist as the more
//           values i converted the less sense they made, so we're gonna need to figure them out
acttable_t COFWeaponBase::m_acttableSecondary[] =
{
	{ ACT_MP_STAND_IDLE, ACT_MP_STAND_SECONDARY, false },
	{ ACT_MP_CROUCH_IDLE, ACT_MP_CROUCH_SECONDARY, false },
	{ ACT_MP_RUN, ACT_MP_RUN_SECONDARY, false },
	{ ACT_MP_WALK, ACT_MP_WALK_SECONDARY, false },
	{ ACT_MP_AIRWALK, ACT_MP_AIRWALK_SECONDARY, false },
	{ ACT_MP_CROUCHWALK, ACT_MP_CROUCHWALK_SECONDARY, false },
	{ ACT_MP_JUMP, ACT_MP_JUMP_SECONDARY, false },
	{ ACT_MP_JUMP_START, ACT_MP_JUMP_START_SECONDARY, false },
	{ ACT_MP_JUMP_FLOAT, ACT_MP_JUMP_FLOAT_SECONDARY, false },
	{ ACT_MP_JUMP_LAND, ACT_MP_JUMP_LAND_SECONDARY, false },
	{ ACT_MP_SWIM, ACT_MP_SWIM_SECONDARY, false },
	{ ACT_MP_DOUBLEJUMP_CROUCH, ACT_MP_DOUBLEJUMP_CROUCH_SECONDARY, false },

	{ ACT_MP_ATTACK_STAND_PRIMARYFIRE, ACT_MP_ATTACK_STAND_SECONDARY, false },
	{ ACT_MP_ATTACK_CROUCH_PRIMARYFIRE, ACT_MP_ATTACK_CROUCH_SECONDARY, false },
	{ ACT_MP_ATTACK_SWIM_PRIMARYFIRE, ACT_MP_ATTACK_SWIM_SECONDARY, false },
	{ ACT_MP_ATTACK_AIRWALK_PRIMARYFIRE, ACT_MP_ATTACK_AIRWALK_SECONDARY, false },

	{ ACT_MP_RELOAD_STAND, ACT_MP_RELOAD_STAND_SECONDARY, false },
	{ ACT_MP_RELOAD_STAND_LOOP, ACT_MP_RELOAD_STAND_SECONDARY_LOOP, false },
	{ ACT_MP_RELOAD_STAND_END, ACT_MP_RELOAD_STAND_SECONDARY_END, false },
	{ ACT_MP_RELOAD_CROUCH, ACT_MP_RELOAD_CROUCH_SECONDARY, false },
	{ ACT_MP_RELOAD_CROUCH_LOOP, ACT_MP_RELOAD_CROUCH_SECONDARY_LOOP, false },
	{ ACT_MP_RELOAD_CROUCH_END, ACT_MP_RELOAD_CROUCH_SECONDARY_END, false },
	{ ACT_MP_RELOAD_SWIM, ACT_MP_RELOAD_SWIM_SECONDARY, false },
	{ ACT_MP_RELOAD_SWIM_LOOP, ACT_MP_RELOAD_SWIM_SECONDARY_LOOP, false },
	{ ACT_MP_RELOAD_SWIM_END, ACT_MP_RELOAD_SWIM_SECONDARY_END, false },
	{ ACT_MP_RELOAD_AIRWALK, ACT_MP_RELOAD_AIRWALK_SECONDARY, false },
	{ ACT_MP_RELOAD_AIRWALK_LOOP, ACT_MP_RELOAD_AIRWALK_SECONDARY_LOOP, false },
	{ ACT_MP_RELOAD_AIRWALK_END, ACT_MP_RELOAD_AIRWALK_SECONDARY_END, false },

	{ ACT_MP_GESTURE_FLINCH, ACT_MP_GESTURE_FLINCH_SECONDARY, false },

	// offset by 46
	{ ACT_MP_GRENADE1_DRAW, ACT_MP_SECONDARY_GRENADE1_DRAW, false },
	{ ACT_MP_GRENADE1_IDLE, ACT_MP_SECONDARY_GRENADE1_IDLE, false },
	{ ACT_MP_GRENADE1_ATTACK, ACT_MP_SECONDARY_GRENADE1_ATTACK, false },
	{ ACT_MP_GRENADE2_DRAW, ACT_MP_SECONDARY_GRENADE2_DRAW, false },
	{ ACT_MP_GRENADE2_IDLE, ACT_MP_SECONDARY_GRENADE2_IDLE, false },
	{ ACT_MP_GRENADE2_ATTACK, ACT_MP_SECONDARY_GRENADE2_ATTACK, false },

	// this is probably right?, makes sense at least
	// we also dont need to worry about it that much, grenades were cut
	{ ACT_MP_ATTACK_STAND_GRENADE, ACT_MP_ATTACK_STAND_GRENADE, false },
	{ ACT_MP_ATTACK_CROUCH_GRENADE, ACT_MP_ATTACK_STAND_GRENADE, false },
	{ ACT_MP_ATTACK_SWIM_GRENADE, ACT_MP_ATTACK_STAND_GRENADE, false },
	{ ACT_MP_ATTACK_AIRWALK_GRENADE, ACT_MP_ATTACK_STAND_GRENADE, false },

	{ ACT_MP_GESTURE_VC_HANDMOUTH, ACT_MP_GESTURE_VC_HANDMOUTH_SECONDARY, false },
	{ ACT_MP_GESTURE_VC_FINGERPOINT, ACT_MP_GESTURE_VC_FINGERPOINT_SECONDARY, false },
	{ ACT_MP_GESTURE_VC_FISTPUMP, ACT_MP_GESTURE_VC_FISTPUMP_SECONDARY, false },
	{ ACT_MP_GESTURE_VC_THUMBSUP, ACT_MP_GESTURE_VC_THUMBSUP_SECONDARY, false },
	{ ACT_MP_GESTURE_VC_NODYES, ACT_MP_GESTURE_VC_NODYES_SECONDARY, false },
	{ ACT_MP_GESTURE_VC_NODNO, ACT_MP_GESTURE_VC_NODNO_SECONDARY, false },
};

acttable_t COFWeaponBase::m_acttablePrimary[] =
{
	{ ACT_MP_STAND_IDLE, ACT_MP_STAND_PRIMARY, false },
	{ ACT_MP_CROUCH_IDLE, ACT_MP_CROUCH_PRIMARY, false },
	{ ACT_MP_DEPLOYED, ACT_MP_DEPLOYED_PRIMARY, false },
	{ ACT_MP_CROUCH_DEPLOYED, ACT_MP_CROUCHWALK_DEPLOYED, false }, // might be wrong?
	{ ACT_MP_CROUCH_DEPLOYED_IDLE, ACT_MP_CROUCH_DEPLOYED_IDLE, false }, // might be wrong?
	{ ACT_MP_RUN, ACT_MP_RUN_PRIMARY, false },
	{ ACT_MP_WALK, ACT_MP_WALK_PRIMARY, false },
	{ ACT_MP_AIRWALK, ACT_MP_AIRWALK_PRIMARY, false },
	{ ACT_MP_CROUCHWALK, ACT_MP_CROUCHWALK_PRIMARY, false },
	{ ACT_MP_JUMP, ACT_MP_JUMP_PRIMARY, false },
	{ ACT_MP_JUMP_START, ACT_MP_JUMP_START_PRIMARY, false },
	{ ACT_MP_JUMP_FLOAT, ACT_MP_JUMP_FLOAT_PRIMARY, false },
	{ ACT_MP_JUMP_LAND, ACT_MP_JUMP_LAND_PRIMARY, false },
	{ ACT_MP_SWIM, ACT_MP_SWIM_PRIMARY, false },
	{ ACT_MP_SWIM_DEPLOYED, ACT_MP_SWIM_DEPLOYED_PRIMARY, false },
	{ ACT_MP_DOUBLEJUMP_CROUCH, ACT_MP_DOUBLEJUMP_CROUCH_PRIMARY, false },

	{ ACT_MP_ATTACK_STAND_PRIMARYFIRE, ACT_MP_ATTACK_STAND_PRIMARY, false },
	{ ACT_MP_ATTACK_STAND_PRIMARYFIRE_DEPLOYED, ACT_MP_ATTACK_STAND_PRIMARY_DEPLOYED, false },
	{ ACT_MP_ATTACK_CROUCH_PRIMARYFIRE, ACT_MP_ATTACK_CROUCH_PRIMARY, false },
	{ ACT_MP_ATTACK_CROUCH_PRIMARYFIRE_DEPLOYED, ACT_MP_ATTACK_CROUCH_PRIMARY_DEPLOYED, false },
	{ ACT_MP_ATTACK_SWIM_PRIMARYFIRE, ACT_MP_ATTACK_SWIM_PRIMARY, false },
	{ ACT_MP_ATTACK_AIRWALK_PRIMARYFIRE, ACT_MP_ATTACK_AIRWALK_PRIMARY, false },
	{ ACT_MP_RELOAD_STAND, ACT_MP_RELOAD_STAND_PRIMARY, false },
	{ ACT_MP_RELOAD_STAND_LOOP, ACT_MP_RELOAD_STAND_PRIMARY_LOOP, false },
	{ ACT_MP_RELOAD_STAND_END, ACT_MP_RELOAD_STAND_PRIMARY_END, false },
	{ ACT_MP_RELOAD_CROUCH, ACT_MP_RELOAD_CROUCH_PRIMARY, false },
	{ ACT_MP_RELOAD_CROUCH_LOOP, ACT_MP_RELOAD_CROUCH_PRIMARY_LOOP, false },
	{ ACT_MP_RELOAD_CROUCH_END, ACT_MP_RELOAD_CROUCH_PRIMARY_END, false },
	{ ACT_MP_RELOAD_SWIM, ACT_MP_RELOAD_SWIM_PRIMARY, false },
	{ ACT_MP_RELOAD_SWIM_LOOP, ACT_MP_RELOAD_SWIM_PRIMARY_LOOP, false },
	{ ACT_MP_RELOAD_SWIM_END, ACT_MP_RELOAD_SWIM_PRIMARY_END, false },
	{ ACT_MP_RELOAD_AIRWALK, ACT_MP_RELOAD_AIRWALK_PRIMARY, false },
	{ ACT_MP_RELOAD_AIRWALK_LOOP, ACT_MP_RELOAD_AIRWALK_PRIMARY_LOOP, false },
	{ ACT_MP_RELOAD_AIRWALK_END, ACT_MP_RELOAD_AIRWALK_PRIMARY_END, false },

	{ ACT_MP_GESTURE_FLINCH, ACT_MP_GESTURE_FLINCH_PRIMARY, false },

	// offset by 46
	{ ACT_MP_GRENADE1_DRAW, ACT_MP_PRIMARY_GRENADE1_DRAW, false },
	{ ACT_MP_GRENADE1_IDLE, ACT_MP_PRIMARY_GRENADE1_IDLE, false },
	{ ACT_MP_GRENADE1_ATTACK, ACT_MP_PRIMARY_GRENADE1_ATTACK, false },
	{ ACT_MP_GRENADE2_DRAW, ACT_MP_PRIMARY_GRENADE2_DRAW, false },
	{ ACT_MP_GRENADE2_IDLE, ACT_MP_PRIMARY_GRENADE2_IDLE, false },
	{ ACT_MP_GRENADE2_ATTACK, ACT_MP_PRIMARY_GRENADE2_ATTACK, false },

	{ ACT_MP_ATTACK_STAND_GRENADE, ACT_MP_ATTACK_STAND_GRENADE, false },
	{ ACT_MP_ATTACK_CROUCH_GRENADE, ACT_MP_ATTACK_STAND_GRENADE, false },
	{ ACT_MP_ATTACK_SWIM_GRENADE, ACT_MP_ATTACK_STAND_GRENADE, false },
	{ ACT_MP_ATTACK_AIRWALK_GRENADE, ACT_MP_ATTACK_STAND_GRENADE, false },

	{ ACT_MP_GESTURE_VC_HANDMOUTH, ACT_MP_GESTURE_VC_HANDMOUTH_PRIMARY, false },
	{ ACT_MP_GESTURE_VC_FINGERPOINT, ACT_MP_GESTURE_VC_FINGERPOINT_PRIMARY, false },
	{ ACT_MP_GESTURE_VC_FISTPUMP, ACT_MP_GESTURE_VC_FISTPUMP_PRIMARY, false },
	{ ACT_MP_GESTURE_VC_THUMBSUP, ACT_MP_GESTURE_VC_THUMBSUP_PRIMARY, false },
	{ ACT_MP_GESTURE_VC_NODYES, ACT_MP_GESTURE_VC_NODYES_PRIMARY, false },
	{ ACT_MP_GESTURE_VC_NODNO, ACT_MP_GESTURE_VC_NODNO_PRIMARY, false },
};

acttable_t COFWeaponBase::m_acttableMelee[] =
{
	{ ACT_MP_STAND_IDLE, ACT_MP_STAND_MELEE, false },
	{ ACT_MP_CROUCH_IDLE, ACT_MP_CROUCH_MELEE, false },
	{ ACT_MP_RUN, ACT_MP_RUN_MELEE, false },
	{ ACT_MP_WALK, ACT_MP_WALK_MELEE, false },
	{ ACT_MP_AIRWALK, ACT_MP_AIRWALK_MELEE, false },
	{ ACT_MP_CROUCHWALK, ACT_MP_CROUCHWALK_MELEE, false },
	{ ACT_MP_JUMP, ACT_MP_JUMP_MELEE, false },
	{ ACT_MP_JUMP_START, ACT_MP_JUMP_START_MELEE, false },
	{ ACT_MP_JUMP_FLOAT, ACT_MP_JUMP_FLOAT_MELEE, false },
	{ ACT_MP_JUMP_LAND, ACT_MP_JUMP_LAND_MELEE, false },
	{ ACT_MP_SWIM, ACT_MP_SWIM_MELEE, false },
	{ ACT_MP_DOUBLEJUMP_CROUCH, ACT_MP_DOUBLEJUMP_CROUCH_MELEE, false },

	{ ACT_MP_ATTACK_STAND_PRIMARYFIRE, ACT_MP_ATTACK_STAND_MELEE, false },
	{ ACT_MP_ATTACK_CROUCH_PRIMARYFIRE, ACT_MP_ATTACK_CROUCH_MELEE, false },
	{ ACT_MP_ATTACK_SWIM_PRIMARYFIRE, ACT_MP_ATTACK_SWIM_MELEE, false },
	{ ACT_MP_ATTACK_AIRWALK_PRIMARYFIRE, ACT_MP_ATTACK_AIRWALK_MELEE, false },
	{ ACT_MP_ATTACK_STAND_SECONDARYFIRE, ACT_MP_ATTACK_STAND_MELEE_SECONDARY, false },
	{ ACT_MP_ATTACK_CROUCH_SECONDARYFIRE, ACT_MP_ATTACK_CROUCH_MELEE_SECONDARY, false },
	{ ACT_MP_ATTACK_SWIM_SECONDARYFIRE, ACT_MP_ATTACK_SWIM_MELEE, false },
	{ ACT_MP_ATTACK_AIRWALK_SECONDARYFIRE, ACT_MP_ATTACK_AIRWALK_MELEE, false },

	{ ACT_MP_GESTURE_FLINCH, ACT_MP_GESTURE_FLINCH_MELEE, false },

	{ ACT_MP_GRENADE1_DRAW, ACT_MP_MELEE_GRENADE1_DRAW, false },
	{ ACT_MP_GRENADE1_IDLE, ACT_MP_MELEE_GRENADE1_IDLE, false },
	{ ACT_MP_GRENADE1_ATTACK, ACT_MP_MELEE_GRENADE1_ATTACK, false },
	{ ACT_MP_GRENADE2_DRAW, ACT_MP_MELEE_GRENADE2_DRAW, false },
	{ ACT_MP_GRENADE2_IDLE, ACT_MP_MELEE_GRENADE2_IDLE, false },
	{ ACT_MP_GRENADE2_ATTACK, ACT_MP_MELEE_GRENADE2_ATTACK, false },

	{ ACT_MP_GESTURE_VC_HANDMOUTH, ACT_MP_GESTURE_VC_HANDMOUTH_MELEE, false },
	{ ACT_MP_GESTURE_VC_FINGERPOINT, ACT_MP_GESTURE_VC_FINGERPOINT_MELEE, false },
	{ ACT_MP_GESTURE_VC_FISTPUMP, ACT_MP_GESTURE_VC_FISTPUMP_MELEE, false },
	{ ACT_MP_GESTURE_VC_THUMBSUP, ACT_MP_GESTURE_VC_THUMBSUP_MELEE, false },
	{ ACT_MP_GESTURE_VC_NODYES, ACT_MP_GESTURE_VC_NODYES_MELEE, false },
	{ ACT_MP_GESTURE_VC_NODNO, ACT_MP_GESTURE_VC_NODNO_MELEE, false },
};

acttable_t COFWeaponBase::m_acttableBuilding[] =
{
	{ ACT_MP_STAND_IDLE, ACT_MP_STAND_BUILDING, false },
	{ ACT_MP_CROUCH_IDLE, ACT_MP_CROUCH_BUILDING, false },
	{ ACT_MP_RUN, ACT_MP_RUN_BUILDING, false },
	{ ACT_MP_WALK, ACT_MP_WALK_BUILDING, false },
	{ ACT_MP_AIRWALK, ACT_MP_AIRWALK_BUILDING, false },
	{ ACT_MP_CROUCHWALK, ACT_MP_CROUCHWALK_BUILDING, false },
	{ ACT_MP_JUMP, ACT_MP_JUMP_BUILDING, false },
	{ ACT_MP_JUMP_START, ACT_MP_JUMP_START_BUILDING, false },
	{ ACT_MP_JUMP_FLOAT, ACT_MP_JUMP_FLOAT_BUILDING, false },
	{ ACT_MP_JUMP_LAND, ACT_MP_JUMP_LAND_BUILDING, false },
	{ ACT_MP_SWIM, ACT_MP_SWIM_BUILDING, false },

	{ ACT_MP_ATTACK_STAND_PRIMARYFIRE, ACT_MP_ATTACK_STAND_BUILDING, false },
	{ ACT_MP_ATTACK_CROUCH_PRIMARYFIRE, ACT_MP_ATTACK_CROUCH_BUILDING, false },
	{ ACT_MP_ATTACK_SWIM_PRIMARYFIRE, ACT_MP_ATTACK_SWIM_BUILDING, false },
	{ ACT_MP_ATTACK_AIRWALK_PRIMARYFIRE, ACT_MP_ATTACK_AIRWALK_BUILDING, false },

	{ ACT_MP_ATTACK_STAND_GRENADE, ACT_MP_ATTACK_STAND_GRENADE_BUILDING, false },
	{ ACT_MP_ATTACK_CROUCH_GRENADE, ACT_MP_ATTACK_STAND_GRENADE_BUILDING, false },
	{ ACT_MP_ATTACK_SWIM_GRENADE, ACT_MP_ATTACK_STAND_GRENADE_BUILDING, false },
	{ ACT_MP_ATTACK_AIRWALK_GRENADE, ACT_MP_ATTACK_STAND_GRENADE_BUILDING, false },

	{ ACT_MP_GESTURE_VC_HANDMOUTH, ACT_MP_GESTURE_VC_HANDMOUTH_BUILDING, false },
	{ ACT_MP_GESTURE_VC_FINGERPOINT, ACT_MP_GESTURE_VC_FINGERPOINT_BUILDING, false },
	{ ACT_MP_GESTURE_VC_FISTPUMP, ACT_MP_GESTURE_VC_FISTPUMP_BUILDING, false },
	{ ACT_MP_GESTURE_VC_THUMBSUP, ACT_MP_GESTURE_VC_THUMBSUP_BUILDING, false },
	{ ACT_MP_GESTURE_VC_NODYES, ACT_MP_GESTURE_VC_NODYES_BUILDING, false },
	{ ACT_MP_GESTURE_VC_NODNO, ACT_MP_GESTURE_VC_NODNO_BUILDING, false }
};

acttable_t COFWeaponBase::m_acttablePDA[] =
{
	{ ACT_MP_STAND_IDLE, ACT_MP_STAND_PDA, false },
	{ ACT_MP_CROUCH_IDLE, ACT_MP_CROUCH_PDA, false },
	{ ACT_MP_RUN, ACT_MP_RUN_PDA, false },
	{ ACT_MP_WALK, ACT_MP_WALK_PDA, false },
	{ ACT_MP_AIRWALK, ACT_MP_AIRWALK_PDA, false },
	{ ACT_MP_CROUCHWALK, ACT_MP_CROUCHWALK_PDA, false },
	{ ACT_MP_JUMP, ACT_MP_JUMP_PDA, false },
	{ ACT_MP_JUMP_START, ACT_MP_JUMP_START_PDA, false },
	{ ACT_MP_JUMP_FLOAT, ACT_MP_JUMP_FLOAT_PDA, false },
	{ ACT_MP_JUMP_LAND, ACT_MP_JUMP_LAND_PDA, false },
	{ ACT_MP_SWIM, ACT_MP_SWIM_PDA, false },

	{ ACT_MP_ATTACK_STAND_PRIMARYFIRE, ACT_MP_ATTACK_STAND_PDA, false },
	{ ACT_MP_ATTACK_SWIM_PRIMARYFIRE, ACT_MP_ATTACK_SWIM_PDA, false },

	{ ACT_MP_GESTURE_VC_HANDMOUTH, ACT_MP_GESTURE_VC_HANDMOUTH_PDA, false },
	{ ACT_MP_GESTURE_VC_FINGERPOINT, ACT_MP_GESTURE_VC_FINGERPOINT_PDA, false },
	{ ACT_MP_GESTURE_VC_FISTPUMP, ACT_MP_GESTURE_VC_FISTPUMP_PDA, false },
	{ ACT_MP_GESTURE_VC_THUMBSUP, ACT_MP_GESTURE_VC_THUMBSUP_PDA, false },
	{ ACT_MP_GESTURE_VC_NODYES, ACT_MP_GESTURE_VC_NODYES_PDA, false },
	{ ACT_MP_GESTURE_VC_NODNO, ACT_MP_GESTURE_VC_NODNO_PDA, false }
};

// OFSTATUS: COMPLETE
// trimmed those last ones as i believe their econ related
acttable_t *COFWeaponBase::ActivityList(int &iActivityCount)
{
	int iWpnType = GetOFWpnData().m_iWeaponType;
	acttable_t *pActTable;

	switch (iWpnType)
	{
	case OF_WEAPON_TYPE_PRIMARY:
	default:
		iActivityCount = ARRAYSIZE(m_acttablePrimary);
		pActTable = m_acttablePrimary;
		break;
	case OF_WEAPON_TYPE_SECONDARY:
		iActivityCount = ARRAYSIZE(m_acttableSecondary);
		pActTable = m_acttableSecondary;
		break;
	case OF_WEAPON_TYPE_MELEE:
		iActivityCount = ARRAYSIZE(m_acttableMelee);
		pActTable = m_acttableMelee;
		break;
	case OF_WEAPON_TYPE_BUILDING:
		iActivityCount = ARRAYSIZE(m_acttableBuilding);
		pActTable = m_acttableBuilding;
		break;
	case OF_WEAPON_TYPE_PDA:
		iActivityCount = ARRAYSIZE(m_acttablePDA);
		pActTable = m_acttablePDA;
		break;
	}
	return pActTable;
}

//OFSTATUS: COMPLETE
// ghidra somehow turned 25 characters into 2000+ characters
// gonna have nightmares about this once im old - cherry
void COFWeaponBase::Materialize()
{
	if (IsEffectActive(EF_NODRAW))
	{
		RemoveEffects(EF_NODRAW);
		CBaseAnimating::DoMuzzleFlash();
	}
	//CCollisionProperty::SetSolidFlags((CCollisionProperty *)&this->field_0x170, *(ushort *)&this->field_0x1ac | 8);
	AddSolidFlags(FSOLID_TRIGGER);
	//CBaseEntity::ThinkSet((FuncDef3 *)this, (float)PTR_SUB_Remove_00e34180, (char *)0x0);
	SetThink(&COFWeaponBase::SUB_Remove);
	//CBaseEntity::SetNextThink((CBaseEntity *)this, (*PTR__gpGlobals_00e34080)->curtime + 1.0, (char *)0x0);
	SetNextThink(gpGlobals->curtime + 1, NULL);
}

//OFSTATUS: COMPLETE
void COFWeaponBase::Operator_HandleAnimEvent(animevent_t *pEvent, CBaseCombatCharacter *pOperator)
{
	if ((pEvent->type & AE_TYPE_NEWEVENTSYSTEM) && (pEvent->type == AE_WPN_INCREMENTAMMO))
	{
		IncrementAmmo();
		m_bAnimReload = true;
	}
}

//OFSTATUS: COMPLETE
bool COFWeaponBase::Ready()
{
	//iVar2 = C_BaseAnimating::SelectWeightedSequence((C_BaseAnimating *)this, 0xcb);
	if (CBaseAnimating::SelectWeightedSequence(ACT_VM_IDLE_LOWERED) == ACTIVITY_NOT_AVAILABLE)
	{
		//*(byte *)&this->field_0x6c = *(byte *)&this->field_0x6c & 0xdf;
		RemoveEffects(EF_NODRAW);
	}
	//if (this->m_bLoweredWeapon != false) {
	//	this->m_bLoweredWeapon = false;
	//}
	m_bLoweredWeapon = false;

	//(*this->vtable->C_TFWeaponBase::SendWeaponAnim)(this, 0xad);
	SendWeaponAnim(ACT_VM_IDLE);

	/*
	iVar2 = C_BaseCombatWeapon::GetOwner((C_BaseCombatWeapon *)this);
	pCVar4 = (CStudioHdr *)0x0;
	iVar3 = 0;
	if (iVar2 != 0) {
		iVar3 = __symbol_stub::___dynamic_cast(iVar2, PTR_typeinfo_00f8a488, PTR_typeinfo_00f8a508, 0);
	}
	fVar1 = *(float *)(*(int *)PTR__gpGlobals_00f8a098 + 0xc);
	iVar2 = this->field_0x7e8;
	if ((*(char *)((int)&this->field_0x878 + 1) == '\0') &&
		(pCVar4 = (CStudioHdr *)this->field_0x880, pCVar4 == (CStudioHdr *)0x0)) {
		C_BaseAnimating::LockStudioHdr((C_BaseAnimating *)this);
		pCVar4 = (CStudioHdr *)this->field_0x880;
	}
	*/
	COFPlayer *pPlayer = GetOFPlayerOwner();

	//fVar5 = (float10)C_BaseAnimating::SequenceDuration((C_BaseAnimating *)this, pCVar4, iVar2);
	//*(float *)(iVar3 + 0xc54) = fVar1 + (float)fVar5;
	pPlayer->m_flNextAttack = gpGlobals->curtime + CBaseAnimating::SequenceDuration();

	return true;
}

//OFSTATUS: COMPLETE
bool COFWeaponBase::Lower()
{
	COFWeaponBase::AbortReload();
	//iVar1 = C_BaseAnimating::SelectWeightedSequence((C_BaseAnimating *)this, 0xcb);
	//if (iVar1 == -1) {
	if (CBaseAnimating::SelectWeightedSequence(ACT_VM_IDLE_LOWERED) == ACTIVITY_NOT_AVAILABLE)
	{
		//C_BaseEntity::AddEffects((C_BaseEntity *)this, 0x20);
		AddEffects(EF_NODRAW);
	}
	m_bLoweredWeapon = true;
	//(*this->vtable->C_TFWeaponBase::SendWeaponAnim)(this, 0xcb);
	COFWeaponBase::SendWeaponAnim(ACT_VM_IDLE_LOWERED);

	return true;
}

//OFSTATUS: COMPLETE
// OFTODO: once IsPlayerClass is done, uncomment the below stuff
float COFWeaponBase::GetNextSecondaryAttackDelay()
{
	float fDelay = 0.5;
	
	//COFPlayer *pPlayer = GetOFPlayerOwner();
	//if (pPlayer && COFPlayer::IsPlayerClass(4)) // forth class is demoman?
	//	{
	//		fDelay = 0.1;
	//	}

	return fDelay;
}


//OFSTATUS: COMPLETE
bool COFWeaponBase::CalcIsAttackCriticalHelper()
{
	if (tf_weapon_criticals.GetBool() == false) return false;

	// GetOFPlayerOwner
	COFPlayer *pPlayer = GetOFPlayerOwner();

	if (!pPlayer) return false;

	// CTFPlayerShared::GetCritMult needs to be finished!
	float flCritMult = pPlayer->GetCritMult();

	if (!COFWeaponBase::CanFireCriticalShot()) return false;

	bool bRapidFireCrits = GetOFWpnData().m_WeaponModeInfo[m_iWeaponMode].m_bUseRapidFireCrits;

	if ((bRapidFireCrits) && m_flCritDuration > gpGlobals->curtime) return true;

	int iRandom = 0;

	if (bRapidFireCrits)
	{
		// muliply by the player's damage done over time
		float flCritMultCalc = clamp(TF_WEAPON_CRIT_CHANCE_RAPID * flCritMult, 0.01f, 0.99f);

		// the amount of time crits last
		float flCritDuration = TF_WEAPON_CRIT_DURATION;

		// the crit chance percentage 
		float flCritChance = 1.0f / ((flCritDuration / flCritMultCalc) - flCritDuration);

		// just to keep things randomized
		int iSeed = (pPlayer->entindex() | entindex() << 8) ^ GetPredictionRandomSeed();
		if (iSeed != m_iCritSeed)
		{
			m_iCritSeed = iSeed;
			RandomSeed(iSeed);
		}

		iRandom = RandomInt(0, TF_WEAPON_RANDOM_RANGE - 1.0);
		if (flCritChance * TF_WEAPON_RANDOM_RANGE > iRandom)
		{
			m_flCritDuration = gpGlobals->curtime + TF_WEAPON_CRIT_DURATION;
			return true;
		}
		return false;
	}
	else
	{
		// just to keep things randomized
		int iSeed = (pPlayer->entindex() | entindex() << 8) ^ GetPredictionRandomSeed();
		if (iSeed != m_iCritSeed)
		{
			m_iCritSeed = iSeed;
			RandomSeed(iSeed);
		}

		iRandom = RandomInt(0, TF_WEAPON_RANDOM_RANGE - 1);
		return iRandom < (TF_WEAPON_CRIT_CHANCE_NORMAL * flCritMult) * TF_WEAPON_RANDOM_RANGE;
	}
}

//OFSTATUS: COMPLETE
void COFWeaponBase::GetProjectileFireSetup(COFPlayer *pPlayer, Vector param_2, Vector *param_3, QAngle *param_4, bool param_5, float param_6)
{
	QAngle angles = pPlayer->EyeAngles();

	Vector vecForward, vecRight, vecUp;

	AngleVectors(angles, &vecForward, &vecRight, &vecUp);

	Vector vecShootPos = pPlayer->Weapon_ShootPosition();

	Vector vecEndPos = param_6 * vecForward + vecShootPos;

	trace_t tr;

	if (param_5)
	{
		CTraceFilterSimple tracefilter(pPlayer, COLLISION_GROUP_NONE);

		ITraceFilter *pTraceFilter = NULL;

		CTraceFilterChain traceFilter(&tracefilter, pTraceFilter);
		UTIL_TraceLine(vecShootPos, vecEndPos, MASK_SOLID, &traceFilter, &tr);
	}
	else
	{
		CTraceFilterTeam traceFilter(pPlayer, COLLISION_GROUP_NONE, pPlayer->GetTeamNumber());
		UTIL_TraceLine(vecShootPos, vecEndPos, MASK_SOLID, &traceFilter, &tr);
	}

	*param_3 = (vecUp * param_2.z) + (vecRight * param_2.y) + (vecForward * param_2.x) + vecShootPos;

	if (tr.fraction > 0.1)
	{
		VectorAngles(tr.endpos - *param_3, *param_4);
	}
	else
	{
		VectorAngles(vecEndPos - *param_3, *param_4);
	}
}

//OFSTATUS: COMPLETE
bool COFWeaponBase::CanPerformSecondaryAttack() const
{
    //NOTE: Trimmed code for checking if demo shield
    
    return CBaseCombatWeapon::CanPerformSecondaryAttack();
}

//OFSTATUS: COMPLETE
// energy weapons dont exist, so just use the base class as normal - cherry
void COFWeaponBase::CheckReload()
{
	CBaseCombatWeapon::CheckReload();
}

//OFSTATUS: COMPLETE
// we dont need to worry about attributes and same for energy weapons, so just use the base class - cherry
void COFWeaponBase::FinishReload()
{
	CBaseCombatWeapon::FinishReload();
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

//OFSTATUS: COMPLETE
// trimmed mvm stuff
void COFWeaponBase::CalcIsAttackCritical()
{
	COFPlayer *pPlayer = GetOFPlayerOwner();
	if (!pPlayer) return;

	if (gpGlobals->framecount == m_iLastCritCheck) return;
	m_iLastCritCheck = gpGlobals->framecount;

	//field_0xb17 = 0; // unused? keeping it here just in case - cherry

	if (OFGameRules()->State_Get() == GR_STATE_TEAM_WIN && OFGameRules()->GetWinningTeam() == pPlayer->GetTeamNumber())
	{
		m_bAttackCritical = true;
		return;
	}

	if (AreRandomCritsEnabled())
	{
		m_bAttackCritical = CalcIsAttackCriticalHelper();
	}
}

//OFSTATUS: COMPLETE
bool COFWeaponBase::IsFiring() const
{
    return false;
}

//OFSTATUS: COMPLETE
// trimmed the matchmaking stuff
bool COFWeaponBase::AreRandomCritsEnabled()
{
	return tf_weapon_criticals.GetBool();
}

//OFSTATUS: COMPLETE
bool COFWeaponBase::DefaultReload(int iClipSize1, int iClipSize2, int iActivity)
{
	COFPlayer *pPlayer = GetOFPlayerOwner();
	if (!pPlayer) return false;

	CBaseCombatCharacter *pOwner = GetOwner();
	if (!pOwner) return false;

	bool bPrimaryReload = false;
	bool bSecondaryReload = false;

	// If you don't have clips, then don't try to reload them.
	if ( UsesClipsForAmmo1() )
	{
		// need to reload primary clip?
		int primary = MIN(iClipSize1 - m_iClip1, pOwner->GetAmmoCount(m_iPrimaryAmmoType));
		if (primary != 0)
		{
			bPrimaryReload = true;
		}
	}

	if (UsesClipsForAmmo2())
	{
		// need to reload secondary clip?
		int secondary = MIN(iClipSize2 - m_iClip2, pOwner->GetAmmoCount(m_iSecondaryAmmoType));
		if (secondary != 0)
		{
			bSecondaryReload = true;
		}
	}

	if (!(bPrimaryReload || bSecondaryReload)) return false;

	float flReloadTime;

#ifdef CLIENT_DLL
	if (ShouldPlayClientReloadSound())
	{
		WeaponSound(RELOAD);
	}
#else
	WeaponSound(RELOAD);
#endif

	pPlayer->DoAnimationEvent(PLAYERANIMEVENT_RELOAD, 0);

	if (SendWeaponAnim(iActivity))
	{
		flReloadTime = SequenceDuration() - 0.2;
	}
	else
	{
		if (bSecondaryReload)
		{
			flReloadTime = GetOFWpnData().m_WeaponModeInfo[OF_WEAPON_TYPE_SECONDARY].m_flTimeReload;
		}
		else
		{
			flReloadTime = GetOFWpnData().m_WeaponModeInfo[OF_WEAPON_TYPE_PRIMARY].m_flTimeReload;
		}
	}

	SetReloadTimer(flReloadTime);
	m_bInReload = true;
	return true;
}

//OFSTATUS: COMPLETE
bool COFWeaponBase::IsReloading() const
{
	return m_iReloadStage != OF_RELOAD_STAGE_NONE;
}

//OFSTATUS: COMPLETE
float COFWeaponBase::GetReloadSpeedScale() const
{
    return 1.0f;
}

//OFSTATUS: COMPLETE
bool COFWeaponBase::CheckReloadMisfire() const
{
    return false;
}

//OFSTATUS: COMPLETE
bool COFWeaponBase::CanDrop() const
{
    return false;
}

//OFSTATUS: COMPLETE
bool COFWeaponBase::AllowTaunts() const
{
    return true;
}

//OFSTATUS: COMPLETE
int COFWeaponBase::GetViewModelWeaponRole()
{
	return GetOFWpnData().m_iWeaponType;
}

//OFSTATUS: COMPLETE
bool COFWeaponBase::CanAttack()
{
	COFPlayer *pPlayer = GetOFPlayerOwner();

	if (pPlayer)
	{
		return pPlayer->m_Shared.CanAttack(GetCanAttackFlags());
	}

    return false;
}

//OFSTATUS: COMPLETE
void COFWeaponBase::StartHolsterAnim()
{
	CBaseCombatWeapon::Holster();
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

//OFSTATUS: COMPLETE
bool COFWeaponBase::IsViewModelFlipped()
{
	// Who uses flipped viewmodels anyways?

	// me :^) - cherry

	COFPlayer *pPlayer = GetOFPlayerOwner();
	if (!pPlayer) return false;

#ifdef CLIENT_DLL
	//bVar4 = (bool)*(char *)&this->field_0xabc != (*(int *)(*(int *)(PTR__cl_flipviewmodels_00f8a2ac + 0x1c) + 0x30) != 0);
	if ( m_bFlipViewModel != cl_flipviewmodels.GetBool() )
	{
		return true;
	}
#else
	//bVar4 = *(char *)&this->field_0x61c != *(char *)(piVar3 + 0x7f3);
	if (m_bFlipViewModel != pPlayer->m_bFlipViewModel)
	{
		return true;
	}
#endif

	return false;
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

// OFSTATUS: COMPLETE
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
	if (GetOFWpnData().m_szExplosionSound && GetOFWpnData().m_szExplosionSound[0])
	{
		//CBaseEntity::PrecacheScriptSound((char *)(iVar5 + 0x991));
		CBaseEntity::PrecacheScriptSound(GetOFWpnData().m_szExplosionSound);
	}

	//if (*(char *)(iVar5 + 0x911) != '\0')
	if (GetOFWpnData().m_szBrassModel && GetOFWpnData().m_szBrassModel[0])
	{
		//CBaseEntity::PrecacheModel((char *)(iVar5 + 0x911));
		CBaseEntity::PrecacheModel(GetOFWpnData().m_szBrassModel);
	}

	//uVar2 = (COFWeaponBase::GetMuzzleFlashParticleEffect);
	if (COFWeaponBase::GetMuzzleFlashParticleEffect())
	{
		//uVar2 = (COFWeaponBase::GetMuzzleFlashParticleEffect);
		PrecacheParticleSystem(COFWeaponBase::GetMuzzleFlashParticleEffect());
	}

	//if (*(char *)(iVar5 + 0xa11) != '\0')
	if (GetOFWpnData().m_szExplosionEffect && GetOFWpnData().m_szExplosionEffect[0])
	{
		//PrecacheParticleSystem((char *)(iVar5 + 0xa11));
		DevMsg("Particle: %s\n", GetOFWpnData().m_szExplosionEffect);
		PrecacheParticleSystem(GetOFWpnData().m_szExplosionEffect);
	}

	//if (*(char *)(iVar5 + 0xa91) != '\0')
	if (GetOFWpnData().m_szExplosionEffectPlayer && GetOFWpnData().m_szExplosionEffectPlayer[0])
	{
		//PrecacheParticleSystem((char *)(iVar5 + 0xa91));
		PrecacheParticleSystem(GetOFWpnData().m_szExplosionEffectPlayer);
	}

	//if (*(char *)(iVar5 + 0xb11) != '\0')
	if (GetOFWpnData().m_szExplosionEffectWater && GetOFWpnData().m_szExplosionEffectWater[0])
	{
		//PrecacheParticleSystem((char *)(iVar5 + 0xb11));
		PrecacheParticleSystem(GetOFWpnData().m_szExplosionEffectWater);
	}

	if (GetOFWpnData().m_szTracerParticle && GetOFWpnData().m_szTracerParticle[0])
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

#ifdef CLIENT_DLL
//OFSTATUS: COMPLETE
bool COFWeaponBase::ShouldPredict()
{
	if ( GetOwner() && GetOwner() == C_BasePlayer::GetLocalPlayer() )
	{
		return true;
	}
	return false;
}

//OFSTATUS: COMPLETE
void COFWeaponBase::OnPreDataChanged(DataUpdateType_t updateType)
{
	BaseClass::OnPreDataChanged(updateType);

	m_bOldWeaponReset = m_bWeaponReset;
}

//OFSTATUS: INCOMPLETE
// trimmed econ, that last part at the bottom is unfinished
void COFWeaponBase::OnDataChanged(DataUpdateType_t updateType)
{
	BaseClass::OnDataChanged(updateType);

	if (updateType == DATA_UPDATE_CREATED)
	{
		ListenForGameEvent("localplayer_changeteam");
	}

	if (GetPredictable() && !ShouldPredict())
	{
		ShutdownPredictable();
	}

	if (m_iWorldModelIndex == GetWorldModelIndex())
	{
		m_nSkin = GetSkin();
	}

	if (m_bWeaponReset != m_bOldWeaponReset)
	{
		WeaponReset();
	}

	// hmm, this part got messed up real bad

	// NOTES:
	// piVar2 is the player

	//uVar3 = this->field_0xbf4;
	//if (uVar3 != 0xffffffff)
	//{
	//	iVar6 = (uVar3 & 0xfff) * 0x10;
	//	if ((*(uint *)(*(int *)PTR__g_pEntityList_00f8a1b4 + 8 + iVar6) == uVar3 >> 0xc) &&
	//		(piVar2 = *(int **)(*(int *)PTR__g_pEntityList_00f8a1b4 + 4 + iVar6), piVar2 != (int *)0x0))
	//	{
	//		(**(code **)(*piVar2 + 0x224))(piVar2);
	//	}
	//}
}

//OFSTATUS: INCOMPLETE
// trimmed econ, bottom part unfinished
int COFWeaponBase::GetWorldModelIndex()
{
	COFPlayer *pPlayer = GetOFPlayerOwner();

	if (pPlayer)
	{
		C_OFPlayer *pLocalPlayer = C_OFPlayer::GetLocalOFPlayer();
		if (!pLocalPlayer) return 0;

		//int iLocalTeam = pLocalPlayer->GetTeamNumber();
		//int iTeam = pPlayer->GetTeamNumber();

		//bool bDisguiseCheck = ((LAST_SHARED_TEAM < iLocalTeam) && (iTeam != iLocalTeam));

		// OFTODO: condition system doesnt exist yet, uncomment in the future - cherry

		//if (bDisguiseCheck && pPlayer->m_Shared.InCond(TF_COND_DISGUISED)) //(cVar3 = CTFPlayerShared::InCond((CTFPlayerShared *)(pPlayer + 0x5f3), TF_COND_DISGUISED), cVar3 != '\0'))// &&
		//	(uVar9 = pPlayer[0x63b], uVar9 != 0xffffffff))
		//{
			//iVar6 = (uVar9 & 0xfff) * 0x10;
			//if (((*(uint *)(*(int *)PTR__g_pEntityList_00f8a1b4 + 8 + iVar6) == uVar9 >> 0xc) &&
			//	(this_00 = *(C_TFWeaponBase **)(*(int *)PTR__g_pEntityList_00f8a1b4 + 4 + iVar6),
			//	this_00 != (C_TFWeaponBase *)0x0)) && (this_00 != this))
			//{
			//	bVar5 = (*this_00->vtable->C_TFWeaponBase::GetWorldModelIndex)(this_00);
			//	return (uint)bVar5;
			//}
		//}
	}

	return BaseClass::GetWorldModelIndex();
}

//OFSTATUS: COMPLETE
void COFWeaponBase::FireGameEvent(IGameEvent *event)
{
	const char *eventname = event->GetName();
	if (V_strcmp(eventname, "localplayer_changeteam") == 0)
	{
		UpdateVisibility();
	}
}

//OFSTATUS: COMPLETE
bool COFWeaponBase::ShouldDrawCrosshair()
{
	// OFTODO: custom crosshairs, they don't exist yet, uncomment in the future

	// const char *crosshair_file = cl_crosshair_file.GetString();
	//if (!crosshair_file)
	//{
		return GetOFWpnData().m_WeaponModeInfo[OF_WEAPON_TYPE_PRIMARY].m_bDrawCrosshair;
	//}

	//return true;
}

//OFSTATUS: COMPLETE
void COFWeaponBase::Redraw()
{
	if (ShouldDrawCrosshair())
	{
		if (g_pClientMode->ShouldDrawCrosshair())
		{
			C_BaseCombatWeapon::DrawCrosshair();
		}
	}
}


// Taken from basehlcombatweapon_shared.cpp - cherry

float g_lateralBob;
float g_verticalBob;

//ConVar OF_BOB_CYCLE_MAX("OF_BOB_CYCLE_MAX", "0.8", FCVAR_NOTIFY | FCVAR_REPLICATED, "");
//ConVar OF_BOB("OF_BOB", "0.5", FCVAR_NOTIFY | FCVAR_REPLICATED, "");

#define	OF_BOB_CYCLE_MAX	0.8f
#define	OF_BOB		0.5f

float COFWeaponBase::CalcViewmodelBob(void)
{
	static	float bobtime;
	static	float lastbobtime;
	float	cycle;

	CBasePlayer *player = ToBasePlayer(GetOwner());
	//Assert( player );

	//NOTENOTE: For now, let this cycle continue when in the air, because it snaps badly without it

	if ((!gpGlobals->frametime) || (player == NULL))
	{
		//NOTENOTE: We don't use this return value in our case (need to restructure the calculation function setup!)
		return 0.0f;// just use old value
	}

	//Find the speed of the player
	float speed = player->GetLocalVelocity().Length2D();

	//FIXME: This maximum speed value must come from the server.
	//		 MaxSpeed() is not sufficient for dealing with sprinting - jdw

	speed = clamp(speed, -320, 320);

	float bob_offset = RemapVal(speed, 0, 320, 0.0f, 1.0f);

	bobtime += (gpGlobals->curtime - lastbobtime) * bob_offset;
	lastbobtime = gpGlobals->curtime;

	//Calculate the vertical bob
	cycle = bobtime - (int)(bobtime / OF_BOB_CYCLE_MAX)*OF_BOB_CYCLE_MAX;
	cycle /= OF_BOB_CYCLE_MAX;

	if (cycle < OF_BOB)
	{
		cycle = M_PI * cycle / OF_BOB;
	}
	else
	{
		cycle = M_PI + M_PI*(cycle - OF_BOB) / (1.0 - OF_BOB);
	}

	g_verticalBob = speed*0.005f;
	g_verticalBob = g_verticalBob*0.3 + g_verticalBob*0.4*sin(cycle);

	g_verticalBob = clamp(g_verticalBob, -7.0f, 4.0f);

	//Calculate the lateral bob
	cycle = bobtime - (int)(bobtime / OF_BOB_CYCLE_MAX * 2)*OF_BOB_CYCLE_MAX * 2;
	cycle /= OF_BOB_CYCLE_MAX * 2;

	if (cycle < OF_BOB)
	{
		cycle = M_PI * cycle / OF_BOB;
	}
	else
	{
		cycle = M_PI + M_PI*(cycle - OF_BOB) / (1.0 - OF_BOB);
	}

	g_lateralBob = speed*0.005f;
	g_lateralBob = g_lateralBob*0.3 + g_lateralBob*0.4*sin(cycle);
	g_lateralBob = clamp(g_lateralBob, -7.0f, 4.0f);

	//NOTENOTE: We don't use this return value in our case (need to restructure the calculation function setup!)
	return 0.0f;
}

void COFWeaponBase::AddViewmodelBob(CBaseViewModel *viewmodel, Vector &origin, QAngle &angles)
{
	Vector	forward, right;
	AngleVectors(angles, &forward, &right, NULL);

	CalcViewmodelBob();

	// Apply bob, but scaled down to 40%
	VectorMA(origin, g_verticalBob * 0.1f, forward, origin);

	// Z bob a bit more
	origin[2] += g_verticalBob * 0.1f;

	// bob the angles
	angles[ROLL] += g_verticalBob * 0.5f;
	angles[PITCH] -= g_verticalBob * 0.4f;

	angles[YAW] -= g_lateralBob  * 0.3f;

	VectorMA(origin, g_lateralBob * 0.8f, right, origin);
}

#endif // CLIENT_DLL

//OFSTATUS: COMPLETE
void COFWeaponBase::WeaponReset()
{
#ifdef GAME_DLL

	m_iReloadStage.Set(OF_RELOAD_STAGE_NONE);

	m_bWeaponReset = !m_bWeaponReset;

#else

    UpdateVisibility();

#endif
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

//OFSTATUS: COMPLETE
void COFWeaponBase::CheckRespawn()
{
	return;
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

// OFSTATUS: COMPLETE
int COFWeaponBase::GetSkin()
{
	COFPlayer *pPlayer = GetOFPlayerOwner();
	if (!pPlayer) return 0;

	int iSkin = 0;

	int iTeam = pPlayer->GetTeamNumber();

#ifdef CLIENT_DLL
	COFPlayer *pLocalPlayer = C_OFPlayer::GetLocalOFPlayer();

	int iLocalTeam = pLocalPlayer->GetTeamNumber();

	if ((1 < iLocalTeam) && (iTeam != iLocalTeam))
	{
		//cVar1 = CTFPlayerShared::InCond((CTFPlayerShared *)(piVar4 + 0x5f3), 3);
		//cVar1 = this_00->m_Shared.InCond(3);

		// OFTODO: condition system doesnt exist yet, uncomment in the future - cherry

		/*
		if ((pLocalPlayer != pPlayer) && pPlayer->m_Shared.InCond(TF_COND_DISGUISED))//(cVar1 == 1))
		{
			//local_14 = CTFPlayerShared::GetDisguiseTeam((CTFPlayerShared *)(this_00 + 0x5f3));
			iTeam = pPlayer->m_Shared.GetDisguiseTeam();
		}
		*/
	}
#endif

	int iSkinOverride = GetSkinOverride();

	if (iSkinOverride == -1)
	{
		switch (iTeam)
		{
		case OF_TEAM_RED:
			iSkin = 0;
			break;
		case OF_TEAM_BLUE:
			iSkin = 1;
			break;
		default:
			iSkin = 1;
			break;
		}
	}
	return iSkin;
}

//OFSTATUS: COMPLETE(?)
void COFWeaponBase::HookAttributes()
{
    // Intentionally Blank
}
