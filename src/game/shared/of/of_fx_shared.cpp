// ========= Copyright Open Fortress Developers, CC-BY-NC-SA ============
// Purpose: Networking between server and client Fire effects, mostly a copy of sdk_fx_shared
// Author(s): KaydemonLP
//

#include "cbase.h"
#include "of_fx_shared.h"
#include "of_weapon_base.h"
#include "of_shareddefs.h"

#ifndef CLIENT_DLL
	#include "ilagcompensationmanager.h"
#endif

#ifdef CLIENT_DLL

#include "fx_impact.h"
#include "c_of_player.h"

// this is a cheap ripoff from CBaseCombatWeapon::WeaponSound():
void FX_WeaponSound(
	int iPlayerIndex,
	WeaponSound_t sound_type,
	const Vector &vOrigin,
	COFWeaponInfo *pWeaponInfo )
{

	// If we have some sounds from the weapon classname.txt file, play a random one of them
	const char *shootsound = pWeaponInfo->aShootSounds[ sound_type ]; 
	if ( !shootsound || !shootsound[0] )
		return;

	CBroadcastRecipientFilter filter; // this is client side only
	if ( !te->CanPredict() )
		return;
			
	CBaseEntity::EmitSound( filter, iPlayerIndex, shootsound, &vOrigin ); 
}

class CGroupedSound
{
public:
	string_t m_SoundName;
	Vector m_vPos;
};

CUtlVector<CGroupedSound> g_GroupedSounds;

	
// Called by the ImpactSound function.
void ShotgunImpactSoundGroup( const char *pSoundName, const Vector &vEndPos )
{
	// Don't play the sound if it's too close to another impact sound.
	for ( int i=0; i < g_GroupedSounds.Count(); i++ )
	{
		CGroupedSound *pSound = &g_GroupedSounds[i];

		if ( vEndPos.DistToSqr( pSound->m_vPos ) < 300*300 )
		{
			if ( Q_stricmp( pSound->m_SoundName, pSoundName ) == 0 )
				return;
		}
	}

	// Ok, play the sound and add it to the list.
	CLocalPlayerFilter filter;
	C_BaseEntity::EmitSound( filter, NULL, pSoundName, &vEndPos );

	int j = g_GroupedSounds.AddToTail();
	g_GroupedSounds[j].m_SoundName = pSoundName;
	g_GroupedSounds[j].m_vPos = vEndPos;
}


void StartGroupingSounds()
{
	Assert( g_GroupedSounds.Count() == 0 );
	SetImpactSoundRoute( ShotgunImpactSoundGroup );
}


void EndGroupingSounds()
{
	g_GroupedSounds.Purge();
	SetImpactSoundRoute( NULL );
}

#else

	#include "of_player.h"
	#include "te_firebullets.h"
	
// Server doesn't play sounds anyway.
void StartGroupingSounds() {}
void EndGroupingSounds() {}
void FX_WeaponSound ( int iPlayerIndex,
	WeaponSound_t sound_type,
	const Vector &vOrigin,
	COFWeaponInfo *pWeaponInfo ) {};

#endif



// This runs on both the client and the server.
// On the server, it only does the damage calculations.
// On the client, it does all the effects.
void FX_FireBullets(
	COFWeaponBase *pWeapon,
	int iPlayer,
	const Vector &vOrigin,
	const QAngle &vAngles,
	int iWeaponID,
	int iMode,
	int iSeed,
	float flSpread,
	float flDamage,
	bool bIsCritical
	)
{
	bool bDoEffects = true;

#ifdef CLIENT_DLL
	C_OFPlayer *pPlayer = ToOFPlayer( ClientEntityList().GetBaseEntity( iPlayer ) );
#else
	COFPlayer *pPlayer = ToOFPlayer( UTIL_PlayerByIndex( iPlayer ) );
#endif

	const char * weaponAlias =	WeaponIDToAlias( iWeaponID );

	if ( !weaponAlias )
	{
		DevMsg("FX_FireBullets: weapon alias for ID %i not found\n", iWeaponID );
		return;
	}

	WEAPON_FILE_INFO_HANDLE	hWpnInfo = LookupWeaponInfoSlot( weaponAlias );

	if ( hWpnInfo == GetInvalidWeaponInfoHandle() )
	{
		DevMsg("FX_FireBullets: LookupWeaponInfoSlot failed for weapon %s\n", weaponAlias );
		return;
	}

	COFWeaponInfo *pWeaponInfo = static_cast< COFWeaponInfo* >( GetFileWeaponInfoFromHandle( hWpnInfo ) );

#ifdef CLIENT_DLL
	// Handled by FireProjectile
	// Do the firing animation event.
	if ( pPlayer && !pPlayer->IsDormant() )
	{
		switch( iMode )
		{
			// Uncomment later, currently causes a crash - Kay
			default:
			case OF_WEAPON_MODE_PRIMARY:
			pPlayer->DoAnimationEvent( PLAYERANIMEVENT_ATTACK_PRIMARY );
			break;
			case OF_WEAPON_MODE_SECONDARY:
			pPlayer->DoAnimationEvent( PLAYERANIMEVENT_ATTACK_SECONDARY );
			break;
		}		
	}
#else
	// if this is server code, send the effect over to client as temp entity
	// Dispatch one message for all the bullet impacts and sounds.
	TE_FireBullets( 
		iPlayer,
		vOrigin, 
		vAngles, 
		iWeaponID,
		iMode,
		iSeed,
		flSpread
		);

	bDoEffects = false; // no effects on server
#endif

	iSeed++;

	// Just cast our damage to int for now
	int		iDamage = (int)flDamage;
	int		iAmmoType = pWeaponInfo->iAmmoType;
/*
	Handled by PlayWeaponShootSound
	WeaponSound_t sound_type = SINGLE;

	if ( bDoEffects )
	{
		FX_WeaponSound( iPlayer, sound_type, vOrigin, pWeaponInfo );
	}
*/

	// Fire bullets, calculate impacts & effects

	if ( !pPlayer )
		return;
	
	StartGroupingSounds();

#if !defined (CLIENT_DLL)
	// Move other players back to history positions based on local player's lag
	lagcompensation->StartLagCompensation( pPlayer, pPlayer->GetCurrentCommand() );
#endif

	for ( int iBullet=0; iBullet < pWeaponInfo->m_WeaponModeInfo[iMode].m_iBullets; iBullet++ )
	{
		RandomSeed( iSeed );	// init random system with this seed

		// Get circular gaussian spread.
		float x, y;
		x = RandomFloat( -0.5, 0.5 ) + RandomFloat( -0.5, 0.5 );
		y = RandomFloat( -0.5, 0.5 ) + RandomFloat( -0.5, 0.5 );
	
		iSeed++; // use new seed for next bullet

		pPlayer->FireBullet(
			vOrigin,
			vAngles,
			flSpread,
			iDamage,
			iAmmoType,
			pPlayer,
			bDoEffects,
			x,y );
	}

#if !defined (CLIENT_DLL)
	lagcompensation->FinishLagCompensation( pPlayer );
#endif

	EndGroupingSounds();
}

