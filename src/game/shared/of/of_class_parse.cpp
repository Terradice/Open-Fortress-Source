// ========= Copyright Open Fortress Developers, CC-BY-NC-SA ============
// Purpose: Implementation of ClassData
// Author(s): KaydemonLP
//

#include "cbase.h"
#include "filesystem.h"
#include "of_shareddefs.h"
#include "of_class_parse.h"
#include "of_weapon_base.h"

// RE: GetWeaponID, GetAmmoName and GetBuildableId aren't RE-d yet

void OFPlayerClassData_t::ParseData( KeyValues *kvData )
{
/*
	puVar1 = __nl_symbol_ptr::___stack_chk_guard;
	local_14 = *(int *)__nl_symbol_ptr::___stack_chk_guard;
*/
	V_strncpy( m_szName, kvData->GetString( "name" ), sizeof(m_szName) );
	
	V_strncpy( m_szModelHWM, kvData->GetString( "model_hwm" ), sizeof(m_szModelHWM) );
	
	V_strncpy( m_szModel, kvData->GetString( "model" ), sizeof(m_szModel) );
	
	V_strncpy( m_szViewmodel, kvData->GetString( "model_hands" ), sizeof(m_szViewmodel) );
	
	V_strncpy( m_szNameLocalized, kvData->GetString( "localize_name" ), sizeof(m_szNameLocalized) );
	
	m_flMaxSpeed = kvData->GetFloat( "speed_max" );
	
	m_iMaxHealth = kvData->GetInt( "health_max" );
	m_iMaxArmor = kvData->GetInt( "armor_max" );

	for( int i = 0; i < sizeof(m_iWeaponIDs); i++ )
	{
		m_iWeaponIDs[i] = AliasToWeaponID( kvData->GetString( Shared_VarArgs( "weapon%d", i ) ) );
	}
	
	for( int i = 0; i < sizeof(m_iGrenades); i++ )
	{
		m_iGrenades[i] = AliasToWeaponID( kvData->GetString( Shared_VarArgs( "grenade%d", i ) ) );
	}
	
	KeyValues *kvAmmoMax = kvData->FindKey( "AmmoMax" );
	if( kvAmmoMax )
	{
		for (int i = 1; i < OF_AMMO_COUNT; i++)
		{
			m_iMaxAmmo[i] = kvAmmoMax->GetInt( GetAmmoName(i) );
		}
	}

	for( int i = 0; i < sizeof(m_iBuildable); i++ )
	{
		m_iBuildable[i] = GetBuildableId( kvData->GetString( Shared_VarArgs( "buildable%d", i ), "") );
	}

	m_bDontDoAirwalk = kvData->GetBool( "DontDoAirwalk" );
	m_bDontDoNewJump = kvData->GetBool( "DontDoNewJump" );

	m_flCameraOffsetForward = kvData->GetFloat( "cameraoffset_forward" );
	m_flCameraOffsetRight = kvData->GetFloat( "cameraoffset_right" );
	m_flCameraOffsetUp = kvData->GetFloat( "cameraoffset_up" );
	
	V_strncpy( m_szDeathSounds[0], kvData->GetString( "sound_death", "Player.Death" ), 
	sizeof(m_szDeathSounds[0]) );
	
	V_strncpy( m_szDeathSounds[1], kvData->GetString( "sound_crit_death", "TFPlayer.CritDeath" ), 
	sizeof(m_szDeathSounds[1]) );
	
	V_strncpy( m_szDeathSounds[2], kvData->GetString( "sound_melee_death", "Player.MeleeDeath" ), 
	sizeof(m_szDeathSounds[2]) );
	
	V_strncpy( m_szDeathSounds[3], kvData->GetString( "sound_explosion_death", "Player.ExplosionDeath" ), 
	sizeof(m_szDeathSounds[3]) );

	m_bIsParsed = true;
	/*
	if (*(int *)puVar1 == local_14) 
	{
		return;
	}*/
}

void OFPlayerClassData_t::AddAdditionalPlayerDeathSounds()
{
	CopySoundNameWithModifierToken( m_szDeathSounds[4], m_szDeathSounds[0], sizeof(m_szDeathSounds[4]),"MVM_" );
	CopySoundNameWithModifierToken( m_szDeathSounds[8], m_szDeathSounds[0], sizeof(m_szDeathSounds[8]),"M_MVM_" );

	CopySoundNameWithModifierToken( m_szDeathSounds[5], m_szDeathSounds[1], sizeof(m_szDeathSounds[5]),"MVM_" );
	CopySoundNameWithModifierToken( m_szDeathSounds[9], m_szDeathSounds[1], sizeof(m_szDeathSounds[9]),"M_MVM_" );

	CopySoundNameWithModifierToken( m_szDeathSounds[6], m_szDeathSounds[2], sizeof(m_szDeathSounds[6]),"MVM_" );
	CopySoundNameWithModifierToken( m_szDeathSounds[10], m_szDeathSounds[2], sizeof(m_szDeathSounds[10]),"M_MVM_" );

	CopySoundNameWithModifierToken( m_szDeathSounds[7], m_szDeathSounds[3], sizeof(m_szDeathSounds[7]),"MVM_" );
	CopySoundNameWithModifierToken( m_szDeathSounds[11], m_szDeathSounds[3], sizeof(m_szDeathSounds[11]),"M_MVM_" );
	return;
}

char *OFPlayerClassData_t::GetDeathSound( int iSoundIndex )
{
	return m_szDeathSounds[iSoundIndex];
}

char *OFPlayerClassData_t::GetModelName( void )
{
	return m_szModel;
}

char *GetTFEncryptionKey( void )
{
	return "E2NcUkG2";
}

char *g_szClassFilepaths[] =
{
	"", // UNDEFINED
	"scripts/playerclasses/scout",
	"scripts/playerclasses/sniper",
	"scripts/playerclasses/soldier",
	"scripts/playerclasses/demoman",
	"scripts/playerclasses/medic",
	"scripts/playerclasses/heavyweapons",
	"scripts/playerclasses/pyro",
	"scripts/playerclasses/spy",
	"scripts/playerclasses/engineer",
	"scripts/playerclasses/civilian"
};

// RE: This seems like a weird way of managing this?
// Valve I guess - Kay
COFPlayerClassDataMgr g_OFPlayerClassDataMgr;
COFPlayerClassDataMgr *g_pOFPlayerClassDataMgr = &g_OFPlayerClassDataMgr;

OFPlayerClassData_t *GetPlayerClassData( uint32 iClassIndex )
{
	return g_pOFPlayerClassDataMgr->Get( iClassIndex );
}


bool COFPlayerClassDataMgr::Init( void )
{
	V_strncpy( m_ClassDatabase[0].m_szName, "undefined", sizeof(m_ClassDatabase[0].m_szName) );
	V_strncpy( m_ClassDatabase[0].m_szModel, "models/player/scout.mdl", sizeof(m_ClassDatabase[0].m_szName) );
	V_strncpy( m_ClassDatabase[0].m_szNameLocalized, "undefined", sizeof(m_ClassDatabase[0].m_szName) );
	
	for( int i = 1; i < OF_CLASS_COUNT; i++ )
	{
		if( m_ClassDatabase[i].m_bIsParsed )
			continue;

		KeyValues *kvClassData = ReadEncryptedKVFile( filesystem, g_szClassFilepaths[i], (const unsigned char *)GetTFEncryptionKey() );
		if( kvClassData )
		{
			m_ClassDatabase[i].ParseData( kvClassData );
			kvClassData->deleteThis();
		}
	}

	return true;
}

OFPlayerClassData_t *COFPlayerClassDataMgr::Get( unsigned int iClassIndex )
{
	return &m_ClassDatabase[iClassIndex];
}

// RE: Called somewhere in CSoundEmitterSystem::LevelInitPreEntity()
void COFPlayerClassDataMgr::AddAdditionalPlayerDeathSounds( void )
{
	for( int i = 1; i < sizeof(m_ClassDatabase); i++ )
	{
		m_ClassDatabase[i].AddAdditionalPlayerDeathSounds();
	}
}


// ----------------------------------------------------------------------------- //
// COFWeaponBase tables.
// ----------------------------------------------------------------------------- //

BEGIN_NETWORK_TABLE_NOBASE( COFPlayerClassShared, DT_OFPlayerClassShared )
#ifdef CLIENT_DLL
	RecvPropInt( RECVINFO( m_iClass ) ),
	RecvPropString( RECVINFO( m_szClassIcon ) ),
	RecvPropString( RECVINFO( m_szCustomModel ) ),

	RecvPropVector( RECVINFO( m_vecCustomModelOffset ) ),
	RecvPropQAngles( RECVINFO( m_angCustomModelRotation ) ),
	RecvPropInt( RECVINFO( m_iCustomModelParity ) ),

	RecvPropBool( RECVINFO( m_bCustomModelRotates ) ),
	RecvPropBool( RECVINFO( m_bCustomRotationSet ) ),
	RecvPropBool( RECVINFO( m_bCustomModelVisibleToSelf ) ),
	RecvPropBool( RECVINFO( m_bCustomModelClassAnim ) ),
#else
	SendPropInt( SENDINFO( m_iClass ) ),

	SendPropStringT( SENDINFO( m_szClassIcon ) ),
	SendPropStringT( SENDINFO( m_szCustomModel ) ),
	SendPropInt( SENDINFO( m_iCustomModelParity ), 3, SPROP_UNSIGNED ),

	SendPropVector( SENDINFO( m_vecCustomModelOffset ) ),
	SendPropQAngles( SENDINFO( m_angCustomModelRotation ) ),

	SendPropBool( SENDINFO( m_bCustomModelRotates ) ),
	SendPropBool( SENDINFO( m_bCustomRotationSet ) ),
	SendPropBool( SENDINFO( m_bCustomModelVisibleToSelf ) ),
	SendPropBool( SENDINFO( m_bCustomModelClassAnim ) ),
#endif
END_NETWORK_TABLE()


COFPlayerClassShared::COFPlayerClassShared()
{
	m_iClass = 0;
#ifdef CLIENT_DLL
	m_szClassIcon[0] = '\0';
	m_szCustomModel[0] = '\0';
#else
	m_szClassIcon.Set( NULL_STRING );
	m_szCustomModel.Set( NULL_STRING );
#endif
	// RE: I hate the fact that there is so much bloat for set custom model
	// But apparently some plugins use it so i gotta keep it - Kay
	m_vecCustomModelOffset = vec3_origin;
	m_angCustomModelRotation = vec3_angle;
	m_bCustomModelRotates = false;
	m_iCustomModelParity = 0;

	Reset();
}

bool COFPlayerClassShared::Init( int iClassIndex )
{
	Reset();

	m_iClass = iClassIndex;
#ifdef GAME_DLL
	// RE: Not defined yet, probably sould do that in the shareddefs
	m_szClassIcon.Set( AllocPooledString( g_aRawPlayerClassNamesShort[ m_iClass ] ) );
#else
	V_strncpy( m_szClassIcon, g_aRawPlayerClassNamesShort[ m_iClass ], sizeof( m_szClassIcon ) );
#endif
	return true;
}

void COFPlayerClassShared::Reset( void )
{
	m_iClass = 0;
#ifdef GAME_DLL
	m_szClassIcon.Set( NULL_STRING );

	m_szCustomModel.Set( NULL_STRING );
#else
	m_szClassIcon[0] = '\0';

	m_szCustomModel[0] = '\0';
#endif
	m_vecCustomModelOffset = vec3_origin;
	m_angCustomModelRotation = vec3_angle;
	m_bCustomModelRotates = true;
	m_bCustomRotationSet = false;
	m_bCustomModelVisibleToSelf = true;
	m_bCustomModelClassAnim = false;
	m_iCustomModelParity = 0;
}

bool COFPlayerClassShared::CanBuildObject( int iObjectIndex )
{
	OFPlayerClassData_t *pClassData = GetPlayerClassData( m_iClass );
	for( int i = 0; i < sizeof(pClassData->m_iBuildable); i++ )
	{
		if( pClassData->m_iBuildable[i] == iObjectIndex )
			return true;
	}

	return false;
}

// RE: Originally string_t
// Changed to char * so its the same on both client and server - Kay
const char *COFPlayerClassShared::GetModelName()
{
	string_t szReturnName = m_szCustomModel;
	if( szReturnName == NULL_STRING ) 
		return GetPlayerClassData( m_iClass )->GetModelName();

	return STRING( szReturnName );
}

// RE: Why is this a variable?
// Couldn't it just be hardcoded? - Kay
char *g_HACK_GunslingerEngineerArmsOverride = "models\\weapons\\c_models\\c_engineer_gunslinger.mdl";

const char *COFPlayerClassShared::GetHandModelName( bool bUseGunslinger )
{
	if( !bUseGunslinger ) 
		return GetPlayerClassData( m_iClass )->m_szViewmodel;

	return g_HACK_GunslingerEngineerArmsOverride;
}

#ifdef GAME_DLL
void COFPlayerClassShared::SetCustomModel( const char *szModelName, bool bCustomModelClassAnim )
{
	if( !(szModelName && szModelName[0]) ) 
	{
		m_szCustomModel.Set( NULL_STRING );
		m_vecCustomModelOffset = vec3_origin;
		m_angCustomModelRotation = vec3_angle;
	}
	else 
	{
		CBaseEntity::SetAllowPrecache( true );
		CBaseEntity::PrecacheModel( szModelName, true );
		CBaseEntity::SetAllowPrecache( CBaseEntity::IsPrecacheAllowed() );
		
		m_szCustomModel = AllocPooledString( szModelName );

		m_bCustomModelClassAnim = bCustomModelClassAnim;
	}

	m_iCustomModelParity = ( m_iCustomModelParity + 1 ) & 7;
}
#endif

bool COFPlayerClassShared::CustomModelHasChanged( void )
{
	if( m_iCustomModelParity != m_iOldCustomModelParity )
	{
		m_iOldCustomModelParity = m_iCustomModelParity;
		return true;
	}
	return false;
}