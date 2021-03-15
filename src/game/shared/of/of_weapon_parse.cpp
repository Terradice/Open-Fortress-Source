// ========= Copyright Open Fortress Developers, CC-BY-NC-SA ============
// Purpose: 
// Author(s): Nopey, KaydemonLP
//

#include "cbase.h"
#include <KeyValues.h>
#include "of_weapon_parse.h"
#include "of_shareddefs.h"

FileWeaponInfo_t* CreateWeaponInfo()
{
	return new COFWeaponInfo;
}


COFWeaponInfo::COFWeaponInfo()
{
}


void COFWeaponInfo::Parse( KeyValues *pKeyValuesData, const char *szWeaponName )
{
	BaseClass::Parse( pKeyValuesData, szWeaponName );

	// These have Secondary_ versions, so just in case, made compatability to add new modes
	// Add things that should be changable via modes here
	// example of how this would be accessed pWeapon->GetTFWeapon()->m_WeaponModeInfo[m_iWeaponType].m_something
	for( int i = 0; i < OF_WEAPON_MODE_COUNT; i++ )
	{
		WeaponInfoStruct_t WeaponInfo_t;
		m_WeaponModeInfo.AddToTail(WeaponInfo_t);
		const char *szPrefix = g_aWeaponModePrefix[i];

		m_WeaponModeInfo[i].m_iDamage		= pKeyValuesData->GetInt( Shared_VarArgs("%sDamage", szPrefix), 0 ); // Douglas Adams 1952 - 2001
		m_WeaponModeInfo[i].m_flRange		= pKeyValuesData->GetFloat( Shared_VarArgs("%sRange", szPrefix), 8192.0f );
		m_WeaponModeInfo[i].m_iBullets		= pKeyValuesData->GetInt( Shared_VarArgs("%sBulletsPerShot", szPrefix), 0 );
		m_WeaponModeInfo[i].m_flSpread		= pKeyValuesData->GetFloat( Shared_VarArgs("%sSpread", szPrefix), 0.0f );
		m_WeaponModeInfo[i].m_flPunchAngle	= pKeyValuesData->GetFloat( Shared_VarArgs("%sPunchAngle", szPrefix), 0.0f );

		m_WeaponModeInfo[i].m_flTimeFireDelay 	 = pKeyValuesData->GetFloat( Shared_VarArgs("%sTimeFireDelay", szPrefix), 0.0f );
		m_WeaponModeInfo[i].m_flTimeIdle 		 = pKeyValuesData->GetFloat( Shared_VarArgs("%sTimeIdle", szPrefix), 0.0f );
		m_WeaponModeInfo[i].m_flTimeIdleEmpty 	 = pKeyValuesData->GetFloat( Shared_VarArgs("%sTimeIdleEmpty", szPrefix), 0.0f );
				
		m_WeaponModeInfo[i].m_flTimeReloadStart  = pKeyValuesData->GetFloat( Shared_VarArgs("%sTimeReloadStart", szPrefix), 0.0f );
		m_WeaponModeInfo[i].m_flTimeReload 		 = pKeyValuesData->GetFloat( Shared_VarArgs("%sTimeReload", szPrefix), 0.0f );
				
		m_WeaponModeInfo[i].m_bDrawCrosshair	 = pKeyValuesData->GetBool( Shared_VarArgs("%sDrawCrosshair", szPrefix), true );
		m_WeaponModeInfo[i].m_iAmmoPerShot		 = pKeyValuesData->GetInt( Shared_VarArgs("%sAmmoPerShot", szPrefix), 1 );
					
		m_WeaponModeInfo[i].m_bUseRapidFireCrits = pKeyValuesData->GetBool( Shared_VarArgs("%sUseRapidFireCrits", szPrefix), true );

		// TODO: Finish this, basically go through a ProjectileType char pointer string array and assign the proper int value
		const char *szProjectileType = pKeyValuesData->GetString("ProjectileType", "projectile_none" );
		m_WeaponModeInfo[i].m_iProjectileType = OF_PROJECTILE_TYPE_NONE;
		for( int y = 0; y < OF_PROJECTILE_TYPE_COUNT; y++ )
		{
			if( FStrEq( szProjectileType, g_aProjectileTypeNames[y] ) )
				m_WeaponModeInfo[i].m_iProjectileType = y;
		}

		m_WeaponModeInfo[i].m_flProjectileSpeed	 = pKeyValuesData->GetFloat( Shared_VarArgs("%sProjectileSpeed", szPrefix), 0.0f ); // Maybe add a min speed 

		m_WeaponModeInfo[i].m_flSmackDelay = pKeyValuesData->GetFloat( Shared_VarArgs("%sSmackDelay", szPrefix), 0.2f );
	}

	// Brass.
	m_bDoInstantEjectBrass = pKeyValuesData->GetBool( "DoInstantEjectBrass", true );
	Q_strncpy( m_szBrassModel, pKeyValuesData->GetString("BrassModel"), sizeof(m_szBrassModel) );
	
	const char *pWeaponType = pKeyValuesData->GetString( "WeaponType", "" );
	
	// Mainly used for animations
	if( FStrEq(pWeaponType, "primary") )
		m_iWeaponType = OF_WEAPON_TYPE_PRIMARY;
	else if( FStrEq(pWeaponType, "secondary") )
		m_iWeaponType = OF_WEAPON_TYPE_SECONDARY;
	else if( FStrEq(pWeaponType, "melee") )
		m_iWeaponType = OF_WEAPON_TYPE_MELEE;
	else if( FStrEq(pWeaponType, "grenade") )
		m_iWeaponType = OF_WEAPON_TYPE_GRENADE;
	else if( FStrEq(pWeaponType, "building") )
		m_iWeaponType = OF_WEAPON_TYPE_BUILDING;
	else if( FStrEq(pWeaponType, "pda") )
		m_iWeaponType = OF_WEAPON_TYPE_PDA;
	else if( FStrEq(pWeaponType, "item1") )
		m_iWeaponType = OF_WEAPON_TYPE_ITEM1;
	else if( FStrEq(pWeaponType, "item2") )
		m_iWeaponType = OF_WEAPON_TYPE_ITEM2;
	
	// Explosion stuff, ie Grenade timers and explosion radius
	m_bGrenade = pKeyValuesData->GetBool( "Grenade", false );
	m_flDamageRadius = pKeyValuesData->GetFloat( "DamageRadius" );
	m_flPrimerTime = pKeyValuesData->GetFloat( "PrimerTime" );
	
	m_bPlayGrenTimer = pKeyValuesData->GetBool( "PlayGrenTimer", true );
	m_bLowerMainWeapon = pKeyValuesData->GetBool( "LowerMainWeapon", true );
	
	// Why do we have this check? To disable team skins we can simply not add them to the model
	// only added because Live TF2 has it - Kay
	m_bVMHasTeamSkins = pKeyValuesData->GetBool( "HasTeamSkins_Viewmodel", false );
	m_bWMHasTeamSkins = pKeyValuesData->GetBool( "HasTeamSkins_Worldmodel", false );
	
	// Still here from the beta, :>
	Q_strncpy( m_szMuzzleFlashModel, pKeyValuesData->GetString("MuzzleFlashModel"), sizeof(m_szMuzzleFlashModel) );
	m_flMuzzleFlashModelDuration = pKeyValuesData->GetFloat( "MuzzleFlashModelDuration", 0.2f );
	
	// Shoot effects woosh *O*
	Q_strncpy( m_szMuzzleFlashParticle, pKeyValuesData->GetString("MuzzleFlashParticleEffect"), sizeof(m_szMuzzleFlashParticle) );
	Q_strncpy( m_szTracerParticle, pKeyValuesData->GetString("TracerEffect"), sizeof(m_szTracerParticle) );
	
	// Explosion effects
	Q_strncpy( m_szExplosionSound, pKeyValuesData->GetString("ExplosionSound"), sizeof(m_szExplosionSound) );
	Q_strncpy( m_szExplosionEffect, pKeyValuesData->GetString("ExplosionEffect"), sizeof(m_szExplosionEffect) );
	Q_strncpy( m_szExplosionEffectPlayer, pKeyValuesData->GetString("ExplosionPlayerEffect"), sizeof(m_szExplosionEffectPlayer) );
	Q_strncpy( m_szExplosionEffectWater, pKeyValuesData->GetString("ExplosionWaterEffect"), sizeof(m_szExplosionEffectWater) );
	
	m_bDontDrop = pKeyValuesData->GetBool( "DontDrop", false );

	const char *pAnimEx = pKeyValuesData->GetString( "PlayerAnimationExtension", "mp5" );
	Q_strncpy( m_szAnimExtension, pAnimEx, sizeof( m_szAnimExtension ) );
}


