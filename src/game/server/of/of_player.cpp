// ========= Copyright Open Fortress Developers, CC-BY-NC-SA ============
// Purpose: Implementation of CTFPlayer
// Author(s): Fenteale
//

// TODO: Check against tf2 server.dylib! Much lacking functionality. -@Nopey
// (Many functions are copied from sdk_player.cpp. Please mark any that match tf2 server.dylib)

#include "cbase.h"
#include "of_shareddefs.h"
#include "of_player.h"
#include "datacache/imdlcache.h"
#include "of_player_shared.h"
#include "of_playeranimstate.h"
#include "tier0/vprof.h"
#include "entity_ofstart.h"
#include "viewport_panel_names.h"
#include "effect_dispatch_data.h"
#include "te_effect_dispatch.h"
#include "engine/ivdebugoverlay.h"
#include "of_weapon_base.h"
#include "of_gamerules.h"
#include "of_team.h"

EHANDLE g_pLastSpawnPoints[OF_TEAM_COUNT];

ConVar sv_motd_unload_on_dismissal( "sv_motd_unload_on_dismissal", "0", 0, "If enabled, the MOTD contents will be unloaded when the player closes the MOTD." );
ConVar tf_playerstatetransitions( "tf_playerstatetransitions", "-2", FCVAR_DEVELOPMENTONLY, "tf_playerstatetransitions <ent index or -1 for all>. Show player state transitions." );

// -------------------------------------------------------------------------------- //
// Player animation event. Sent to the client when a player fires, jumps, reloads, etc..
// -------------------------------------------------------------------------------- //

class CTEPlayerAnimEvent : public CBaseTempEntity
{
public:
	DECLARE_CLASS( CTEPlayerAnimEvent, CBaseTempEntity );
	DECLARE_SERVERCLASS();

					CTEPlayerAnimEvent( const char *name ) : CBaseTempEntity( name )
					{
					}

	CNetworkHandle( CBasePlayer, m_hPlayer );
	CNetworkVar( int, m_iEvent );
	CNetworkVar( int, m_nData );
};

IMPLEMENT_SERVERCLASS_ST_NOBASE( CTEPlayerAnimEvent, DT_TEPlayerAnimEvent )
	SendPropEHandle( SENDINFO( m_hPlayer ) ),
	SendPropInt( SENDINFO( m_iEvent ), Q_log2( PLAYERANIMEVENT_COUNT ) + 1, SPROP_UNSIGNED ),
	SendPropInt( SENDINFO( m_nData ), 32 )
END_SEND_TABLE()

static CTEPlayerAnimEvent g_TEPlayerAnimEvent( "PlayerAnimEvent" );

void TE_PlayerAnimEvent( CBasePlayer *pPlayer, PlayerAnimEvent_t event, int nData )
{
	CPVSFilter filter( (const Vector&)pPlayer->EyePosition() );
	
	g_TEPlayerAnimEvent.m_hPlayer = pPlayer;
	g_TEPlayerAnimEvent.m_iEvent = event;
	g_TEPlayerAnimEvent.m_nData = nData;
	g_TEPlayerAnimEvent.Create( filter, 0 );
}

// implementation of COFPlayer begin

int SendProxyArrayLength_PlayerObjects(const void *pStruct, int objectID)
{
	//COFPlayer *pPlayer = (COFPlayer*)pStruct;
	//return pPlayer->GetObjectCount();

	// REMOVE LATER
	return 0;
}

void SendProxy_PlayerObjectList(const SendProp *pProp, const void *pStruct, const void *pData, DVariant *pOut, int iElement, int objectID)
{
	// OFTODO: work on objects! (ex: sentrys)
	//COFPlayer *pPlayer = (COFPlayer*)pStruct;

	//CBaseObject *pObject = pPlayer->GetObject();

	//EHandle hObject = pObject;

	//SendProxy_EHandleToInt(pProp, pStruct, &hObject, pOut, iElement, objectID);
}

void *SendProxy_SendNonLocalDataTable(const SendProp *pProp, const void *pStruct, const void *pVarData, CSendProxyRecipients *pRecipients, int objectID)
{
	pRecipients->SetAllRecipients();
	pRecipients->ClearRecipient(objectID - 1);
	return (void *)pVarData;
}
REGISTER_SEND_PROXY_NON_MODIFIED_POINTER(SendProxy_SendNonLocalDataTable);

LINK_ENTITY_TO_CLASS( player, COFPlayer );

// Local Exclusive
BEGIN_SEND_TABLE_NOBASE(COFPlayer, DT_OFLocalPlayerExclusive)
	// for some reason the origin is split in SendPropVectorXY and SendPropFloat ??? thats dumb so here's a vector instead
	SendPropVector( SENDINFO(m_vecOrigin), -1, SPROP_NOSCALE | SPROP_CHANGES_OFTEN, 0.0, HIGH_DEFAULT, SendProxy_Origin ),
	//SendPropArray2(
	//SendProxyArrayLength_PlayerObjects,
	//SendPropInt("player_object_array_element", 0, -1, NUM_NETWORKED_EHANDLE_BITS, SPROP_UNSIGNED, SendProxy_PlayerObjectList), 6, 0, ""),
	SendPropFloat(SENDINFO_VECTORELEM(m_angEyeAngles, 0), 8, SPROP_CHANGES_OFTEN, -90.0, 90.0),
	SendPropAngle(SENDINFO_VECTORELEM(m_angEyeAngles, 1), 10, SPROP_CHANGES_OFTEN),
END_SEND_TABLE()

// Non Local Exclusive
BEGIN_SEND_TABLE_NOBASE(COFPlayer, DT_OFNonLocalPlayerExclusive)
	// for some reason the origin is split in SendPropVectorXY and SendPropFloat ??? thats dumb so here's a vector instead
	SendPropVector(SENDINFO(m_vecOrigin), -1, SPROP_COORD_MP_LOWPRECISION | SPROP_CHANGES_OFTEN, 0.0, HIGH_DEFAULT, SendProxy_Origin),
	SendPropFloat(SENDINFO_VECTORELEM(m_angEyeAngles, 0), 8, SPROP_CHANGES_OFTEN, -90.0, 90.0),
	SendPropAngle(SENDINFO_VECTORELEM(m_angEyeAngles, 1), 10, SPROP_CHANGES_OFTEN),
END_SEND_TABLE()

IMPLEMENT_SERVERCLASS_ST(COFPlayer, DT_OF_Player)
	SendPropDataTable(SENDINFO_DT(m_Class), &REFERENCE_SEND_TABLE(DT_OFPlayerClassShared)),
	SendPropDataTable("oflocaldata", 0, &REFERENCE_SEND_TABLE(DT_OFLocalPlayerExclusive), SendProxy_SendLocalDataTable),
	SendPropDataTable("ofnonlocaldata", 0, &REFERENCE_SEND_TABLE(DT_OFNonLocalPlayerExclusive), SendProxy_SendNonLocalDataTable),
END_SEND_TABLE()

BEGIN_DATADESC( COFPlayer )
END_DATADESC()

COFPlayer::COFPlayer()
{
	m_PlayerAnimState = CreatePlayerAnimState(this);

	SetArmorValue(10);

	UseClientSideAnimation();

	m_bFlipViewModel = false;
}

bool COFPlayer::m_bOFPlayerNeedsPrecache { true };

COFPlayer* COFPlayer::CreatePlayer( const char * name, edict_t* pEdict)
{
	CBasePlayer::s_PlayerEdict = pEdict;
	return (COFPlayer *)CreateEntityByName(name, -1);
}

//OFSTATUS: Incomplete, some variables are supposed to be set here
void COFPlayer::InitialSpawn()
{
	BaseClass::InitialSpawn();

//	CAttributeList::SetManager( ( CAttributeList* )( this + 653 ), ( CAttributeManager* )( this + 1120 ) );

	StateEnter( TF_STATE_WELCOME );
//	UpdateInventory( true );

//	Here the event "player_initial_spawn" should be fired
}

//OFSTATUS: Incomplete, must implement all states
void COFPlayer::StateEnter(int state)
{
	if ( m_Shared.GetPlayerState() != state )
		m_Shared.SetPlayerState(state);

	if ( tf_playerstatetransitions.GetInt() != -2 )
	{
		if ( tf_playerstatetransitions.GetInt() == -1 || tf_playerstatetransitions.GetInt() == entindex() )
			Msg( "ShowStateTransitions: entering \'%s\'\n", sz_OFPlayerState[state] );
	}

	switch ( state )
	{
	case TF_STATE_WELCOME:
		StateEnterWELCOME();
		break;
	default:
		break;
	}
}

//OFSTATUS: Incomplete, info_observer_point needs to be added
void COFPlayer::StateEnterWELCOME()
{
	CBaseEntity* pOT = gEntList.FindEntityByClassname( NULL, "info_observer_point" );
	while ( pOT != NULL )
	{
		if ( IsValidObserverTarget( pOT ) )
		{
			SetObserverTarget( pOT );
			break;
		}

		pOT = gEntList.FindEntityByClassname( pOT, "info_observer_point" );
	}

	StartObserverMode( OBS_MODE_FIXED );
	SetMoveType( MOVETYPE_NONE );
	SetSolidFlags( GetSolidFlags() | SOLID_OBB_YAW );
	AddEffects( 48 );

	char title[128];

	if ( UTIL_GetActiveHolidayString() )
	{
		V_snprintf( title, sizeof( title ), "#TF_Welcome_%s", UTIL_GetActiveHolidayString() );
	}
	else
	{
		V_snprintf( title, sizeof( title ), "#TF_Welcome" );
	}

	// open info panel on client showing MOTD:
	KeyValues* data = new KeyValues( "data" );
	data->SetString( "title", title );		// info panel title
	data->SetString( "type", "1" );			// show userdata from stringtable entry
	data->SetString( "msg", "motd" );		
	data->SetString( "msg_fallback", "motd_text" );
	data->SetBool( "unload", sv_motd_unload_on_dismissal.GetBool() );

	ShowViewPortPanel( PANEL_INFO, true, data );

	data->deleteThis();
}

//OFSTATUS: Incomplete
void COFPlayer::Spawn()
{
    Precache(); //this wasnt in kay's code.  maybe its called automatically?
    
    //start line 276 CTFPlayer::Spawn
    SetModelScale( 1.0f );
    UpdateModel(); //line 277 - 284 CTFPlayer::Spawn() is inlined UpdateModel()
    SetMoveType( MOVETYPE_WALK );
    BaseClass::Spawn();
    //end line 286 CTFPlayer::Spawn
	CreateViewModel(1);
	GetViewModel(1)->SetModel("");
    RemoveSolidFlags( FSOLID_NOT_SOLID );
    m_hRagdoll = NULL;

    if (m_Shared.GetPlayerState() == TF_STATE_WELCOME)
    {
        StateEnterWELCOME();
    }

	if (m_Shared.GetPlayerState() == TF_STATE_DYING)
	{
		StateEnter(TF_STATE_ACTIVE);
	}

	if (m_Shared.GetPlayerState() == TF_STATE_ACTIVE)
	{
		//if (m_Shared.InCond(TF_COND_DISGUISED))
		//{
		//	m_Shared.RemoveDisguise();
		//}

		EmitSound("Player.Spawn");

		//field_0x20c0
		InitClass();

		//m_Shared.RemoveAllCond();

		//m_Shared.AddCond(TF_COND_TEAM_GLOWS)

		m_nSkin.Set(GetTeamNumber() - 2);

		SetNextAttack(gpGlobals->curtime + 1.0);

		DoAnimationEvent(PLAYERANIMEVENT_SPAWN);
	}
}

void COFPlayer::PostThink()
{
	BaseClass::PostThink();

	QAngle angle = GetLocalAngles();
	angle[PITCH] = 0;
	SetLocalAngles(angle);

	m_angEyeAngles = EyeAngles();
	m_PlayerAnimState->Update(m_angEyeAngles[YAW], m_angEyeAngles[PITCH]);

	// this is where it handles those taunt kills as DoTauntAttack is huge
	if (m_fTauntKillTime && m_fTauntKillTime < gpGlobals->curtime)
	{
		m_fTauntKillTime = 0; // field_0x1fa4
		//DoTauntAttack();
	}

	// coaching, mvm, and halloween past here - ignore!
}

//OFSTATUS: Incomplete, placeholder
void COFPlayer::ForceRespawn()
{
    StateEnter( TF_STATE_ACTIVE );
	Spawn();
    //BaseClass::ForceRespawn();
}

//OFSTATUS: Complete
void COFPlayer::InitClass()
{
	SetArmorValue( GetPlayerClassData(m_Class.m_iClass)->m_iMaxArmor );

	m_PlayerAnimState->SetRunSpeed( GetPlayerClassData(m_Class.m_iClass)->m_flMaxSpeed );
	m_PlayerAnimState->SetWalkSpeed( GetPlayerClassData(m_Class.m_iClass)->m_flMaxSpeed * 0.5 );

	GiveDefaultItems();

	SetMaxHealth( GetMaxHealth() );
	SetHealth( GetMaxHealth() );

	// TeamFortress_SetSpeed();
}

void COFPlayer::GiveDefaultItems()
{
	OFPlayerClassData_t *pClassData = GetPlayerClassData(m_Class.m_iClass);
	
	//if (GetTeamNumber() == TEAM_SPECTATOR)
	//{
		RemoveAllWeapons();
		RemoveAllAmmo();
		//return;
	//}

	for (int i = 0; i < OF_AMMO_COUNT; i++)
	{
		GiveAmmo(pClassData->m_iMaxAmmo[i], i, true);
	}

	ManageRegularWeapons(pClassData);

	if (!OFGameRules()->IsPlayingMedievalMode())
	{
		//ManageBuilderWeapons(pClassData);
	}

	//m_Shared.RemoveCond(TF_COND_SELECTED_TO_TELEPORT);
	// there's a bunch of other conditions but are for econ weapons
}

// this is FILLED with econ, steamid checks, and who knows what else.
// makes sense since, item servers, so im doing this manually - cherry
void COFPlayer::ManageRegularWeapons(OFPlayerClassData_t *pClassData)
{
	for (int i = 0; i < OF_PLAYER_WEAPON_COUNT; i++)
	{
		int iWeaponID = pClassData->m_iWeaponIDs[i];
		if (iWeaponID != WEAPON_NONE)
		{
			const char *pszWeaponName = WeaponIDToAlias(iWeaponID);
			GiveNamedItem(pszWeaponName);
		}
	}

}

int COFPlayer::GetMaxHealth()
{
	int iRet = GetMaxHealthForBuffing();
	//iVar1 = CAttributeManager::AttribHookValue<int>
    //                (iVar1,"add_maxhealth_nonbuffed",(CBaseEntity *)this,(CUtlVector *)0x0,true);
	if( iRet < 1 ) 
		iRet = 1;
	return iRet;
}

int COFPlayer::GetMaxHealthForBuffing()
{
	OFPlayerClassData_t *pClassData = GetPlayerClassData( m_Class.m_iClass );
	int iHealth = pClassData->m_iMaxHealth;

	return iHealth;
	// RE: Attribute stuff, ignore - Kay
	// iHealth = CAttributeManager::AttribHookValue<int>
	// 	(iHealth, "add_maxhealth", (CBaseEntity *)this, (CUtlVector *)0x0, true);

	//COFWeaponBase *pOFWeapon = GetActiveOFWeapon();
	// RE: These are just all overrides weapons do for max health
	// None of the base weapons use it, ignore for now - Kay
	//if( pOFWeapon )
	//{
	//	uVar2 = (*pTFWeapon->vtable->CTFWeaponBase::GetMaxHealthMod)();
	//	iVar4 = iVar4 + CONCAT31(extraout_var, uVar2);
	//}

	// Re: OK SO this just checks if you're demo and have a sword or whatever
	// We don't have that so i won't go into this
	// if ((((this->m_Class).m_iClass == 4) && (iVar5 = Weapon_OwnsThisID(this, 0x40), iVar5 != 0)) &&
	// 	(piVar6 = (CTFSword *)
	// 	__symbol_stub::___dynamic_cast(iVar5, PTR_typeinfo_00e344b0, PTR_typeinfo_00e34724, 0),
	// 	piVar6 != (CTFSword *)0x0)) {
	// 	iVar5 = (**(code **)(*(int *)piVar6 + 0x794))();
	// 	iVar4 = iVar4 + iVar5;
	// }

	// Re: Runes are the Mannpower powerups
	// IGNOREEEE - Kay
	// iHealth += GetRuneHealthBonus();


	
	//if( m_Shared.InCond( TF_COND_HALLOWEEN_GIANT ) )
	//	return iHealth * tf_halloween_giant_health_scale.GetFloat();


	// RE: This entire thing is for heavies health drain rate on the GRU
	// I am so sorry for whoever needed to code this
	//else{
	//	fVar16 = CAttributeManager::AttribHookValue<float>
	//		(0.0, "mod_maxhealth_drain_rate", (CBaseEntity *)piVar5, (CUtlVector *)0x0, true)
	//		;
	//	if (fVar16 <= 0.0) {
	//		iVar4 = iVar6;
	//		if (*(int *)&this->field_0x2444 < 1) {
	//			if (*(double *)&this->field_0x2448 != -1.0) {
	//				*(undefined4 *)&this->field_0x2444 = 0;
	//				*(undefined4 *)&this->field_0x244c = 0xbff00000;
	//				*(undefined4 *)&this->field_0x2448 = 0;
	//				*(undefined *)&this->field_0x2450 = 1;
	//				*(undefined4 *)&this->field_0x2458 = 0;
	//				*(undefined4 *)&this->field_0x2454 = 0;
	//				*(undefined4 *)&this->field_0x2460 = 0;
	//				*(undefined4 *)&this->field_0x245c = 0;
	//			}
	//		}
	//		else {
	//			if (*(char *)&this->field_0x2450 == '\0') {
	//				if (0.0 < *(double *)&this->field_0x2454) {
	//					*(ulonglong *)&this->field_0x2454 =
	//						(ulonglong)*(double *)&this->field_0x2454 ^ 0x8000000000000000;
	//				}
	//				if (0.0 < *(double *)&this->field_0x245c) {
	//					*(ulonglong *)&this->field_0x245c =
	//						(ulonglong)*(double *)&this->field_0x245c ^ 0x8000000000000000;
	//				}
	//			}
	//			*(undefined *)&this->field_0x2450 = 1;
	//			fVar16 = CAttributeManager::AttribHookValue<float>
	//				(0.0, "mod_maxhealth_drain_rate", (CBaseEntity *)this, (CUtlVector *)0x0,
	//				true);
	//			ppCVar1 = PTR__gpGlobals_00e34080;
	//			if (fVar16 == 0.0) {
	//				*(undefined4 *)&this->field_0x2444 = 0;
	//				*(undefined4 *)&this->field_0x244c = 0xbff00000;
	//				*(undefined4 *)&this->field_0x2448 = 0;
	//				*(undefined *)&this->field_0x2450 = 1;
	//				*(undefined4 *)&this->field_0x2458 = 0;
	//				*(undefined4 *)&this->field_0x2454 = 0;
	//				*(undefined4 *)&this->field_0x2460 = 0;
	//				*(undefined4 *)&this->field_0x245c = 0;
	//			}
	//			else {
	//				iVar4 = *(int *)&this->field_0x2444;
	//				fVar13 = (*PTR__gpGlobals_00e34080)->curtime;
	//				dVar14 = (double)fVar16 * ((double)fVar13 - *(double *)&this->field_0x2448);
	//				dVar15 = *(double *)&this->field_0x2454 + dVar14;
	//				*(double *)&this->field_0x2454 = dVar15;
	//				fVar12 = (float10)__symbol_stub::_floor(dVar15);
	//				iVar9 = (int)fVar12;
	//				iVar10 = iVar4;
	//				if (0 < iVar9) {
	//					*(double *)&this->field_0x2454 = dVar15 - (double)iVar9;
	//					iVar10 = iVar4 - iVar9;
	//					*(int *)&this->field_0x2444 = iVar10;
	//				}
	//				iVar9 = *(int *)&this->field_0x108;
	//				if (iVar6 - iVar4 < 2) {
	//					fVar16 = 1.0;
	//				}
	//				else {
	//					fVar16 = (float)(iVar6 - iVar4);
	//				}
	//				dVar14 = (double)((float)iVar9 / fVar16) * dVar14 + *(double *)&this->field_0x245c;
	//				*(double *)&this->field_0x245c = dVar14;
	//				fVar12 = (float10)__symbol_stub::_floor();
	//				iVar4 = (int)fVar12;
	//				if (0 < iVar4) {
	//					*(double *)&this->field_0x245c = dVar14 - (double)iVar4;
	//					fVar12 = (float10)CTFPlayerShared::GetMaxOverhealMultiplier
	//						((CTFPlayerShared *)&this->field_0x19b8);
	//					iVar10 = CAttributeManager::AttribHookValue<int>
	//						(iVar6, "add_maxhealth_nonbuffed", (CBaseEntity *)this,
	//						(CUtlVector *)0x0, true);
	//					if (iVar10 <= (int)((float)iVar6 * (float)fVar12)) {
	//						iVar10 = (int)((float)iVar6 * (float)fVar12);
	//					}
	//					iVar4 = iVar4 + *(int *)&this->field_0x108;
	//					if (iVar10 < iVar4) {
	//						iVar4 = iVar10;
	//					}
	//					iVar9 = *(int *)&this->field_0x108;
	//					if (*(int *)&this->field_0x108 != iVar4) {
	//						(**(code **)(*(int *)this + 0x1ec))(this, (int *)&this->field_0x108);
	//						*(int *)&this->field_0x108 = iVar4;
	//						iVar9 = iVar4;
	//					}
	//					fVar13 = (*ppCVar1)->curtime;
	//					iVar10 = *(int *)&this->field_0x2444;
	//				}
	//				*(double *)&this->field_0x2448 = (double)fVar13;
	//				iVar4 = 1;
	//				if (iVar6 - iVar10 != 0 && iVar10 <= iVar6) {
	//					iVar4 = iVar6 - iVar10;
	//				}
	//				if (iVar4 == iVar6) {
	//					if ((*(double *)&this->field_0x245c != 0.0) && (*(int *)&this->field_0x108 != iVar9 + 1)
	//						) {
	//						(**(code **)(*(int *)this + 0x1ec))(this, (int *)&this->field_0x108);
	//						*(int *)&this->field_0x108 = iVar9 + 1;
	//					}
	//					*(undefined4 *)&this->field_0x2444 = 0;
	//					*(undefined4 *)&this->field_0x244c = 0xbff00000;
	//					*(undefined4 *)&this->field_0x2448 = 0;
	//					*(undefined *)&this->field_0x2450 = 1;
	//					*(undefined4 *)&this->field_0x2458 = 0;
	//					*(undefined4 *)&this->field_0x2454 = 0;
	//					*(undefined4 *)&this->field_0x2460 = 0;
	//					*(undefined4 *)&this->field_0x245c = 0;
	//					iVar4 = iVar6;
	//				}
	//			}
	//		}
	//	}
	//	else {
	//		iVar4 = 0;
	//		if (*(char *)&this->field_0x2450 != '\0') {
	//			iVar4 = *(int *)(DAT_0108fbc0 + 0x30);
	//			*(undefined *)&this->field_0x2450 = 0;
	//			if (*(double *)&this->field_0x2454 < 0.0) {
	//				*(ulonglong *)&this->field_0x2454 =
	//					(ulonglong)*(double *)&this->field_0x2454 & 0x7fffffffffffffff;
	//			}
	//			if (*(double *)&this->field_0x245c < 0.0) {
	//				*(ulonglong *)&this->field_0x245c =
	//					(ulonglong)*(double *)&this->field_0x245c & 0x7fffffffffffffff;
	//			}
	//		}
	//		dVar14 = *(double *)&this->field_0x2448;
	//		if (dVar14 == -1.0) {
	//			dVar14 = (double)(*PTR__gpGlobals_00e34080)->curtime;
	//			*(double *)&this->field_0x2448 = dVar14;
	//		}
	//		iVar10 = *(int *)(DAT_0108fb64 + 0x30);
	//		iVar9 = *(int *)&this->field_0x2444;
	//		iVar8 = iVar9;
	//		if (iVar9 < iVar6 - iVar10) {
	//			dVar15 = (double)iVar4 +
	//				(double)fVar16 * ((double)(*PTR__gpGlobals_00e34080)->curtime - dVar14);
	//			dVar14 = *(double *)&this->field_0x2454 + dVar15;
	//			*(double *)&this->field_0x2454 = dVar14;
	//			fVar12 = (float10)__symbol_stub::_floor(dVar14);
	//			iVar4 = (int)fVar12;
	//			if (0 < iVar4) {
	//				*(double *)&this->field_0x2454 = dVar14 - (double)iVar4;
	//				iVar8 = iVar4 + iVar9;
	//				*(int *)&this->field_0x2444 = iVar8;
	//			}
	//			iVar4 = *(int *)&this->field_0x108;
	//			iVar9 = iVar6 - iVar9;
	//			if (iVar6 - iVar9 < iVar10) {
	//				iVar9 = iVar10;
	//			}
	//			dVar14 = (double)((float)iVar4 / (float)iVar9) * dVar15 + *(double *)&this->field_0x245c;
	//			*(double *)&this->field_0x245c = dVar14;
	//			fVar12 = (float10)__symbol_stub::_floor(dVar14);
	//			iVar9 = (int)fVar12;
	//			if (0 < iVar9) {
	//				*(double *)&this->field_0x245c = dVar14 - (double)iVar9;
	//				iVar11 = 1;
	//				if (iVar4 - iVar9 != 0 && iVar9 <= iVar4) {
	//					iVar11 = iVar4 - iVar9;
	//				}
	//				if (*(int *)&this->field_0x108 != iVar11) {
	//					(**(code **)(*(int *)this + 0x1ec))(this, &this->field_0x108);
	//					*(int *)&this->field_0x108 = iVar11;
	//					iVar8 = *(int *)&this->field_0x2444;
	//				}
	//			}
	//		}
	//		iVar4 = iVar6 - iVar8;
	//		if (iVar6 - iVar8 < iVar10) {
	//			iVar4 = iVar10;
	//		}
	//		*(double *)&this->field_0x2448 = (double)(*PTR__gpGlobals_00e34080)->curtime;
	//	}
	//}
}


void COFPlayer::DoAnimationEvent( PlayerAnimEvent_t event, int nData )
{
	MDLCACHE_CRITICAL_SECTION();
	m_PlayerAnimState->DoAnimationEvent( event, nData );
	TE_PlayerAnimEvent( this, event, nData );	// Send to any clients who can see this guy.
}

//OFSTATUS: Incomplete, and low priority
void COFPlayer::PreCacheKart()
{
}

//OFSTATUS: Incomplete, and low priority
void COFPlayer::PreCacheMvM()
{
}


//OFSTATUS: Nearly complete, only missing vprof(but that doesn't make any difference ingame)
void COFPlayer::Precache()
{
    //VPROF_SCOPE_BEGIN("CTFPlayer::Precache"); //Maybe, this could also be something else
    /*uint *puVar1;
    int iVar2;
    undefined *puVar3;
    char cVar4;
    int iVar5;
    char **ppcVar6;
    int iVar7;
    undefined *puVar8;
  
    puVar3 = _g_VProfCurrentProfile;
    iVar2 = *(int *)(_g_VProfCurrentProfile + 0x100c);
    if (iVar2 != 0) {
    iVar7 = *(int *)(_g_VProfCurrentProfile + 0x19b8);
    iVar5 = _ThreadGetCurrentId();
    if (iVar7 == iVar5) {
        ppcVar6 = *(char ***)(puVar3 + 0x1014);
        if (*ppcVar6 != "CTFPlayer::Precache") {
        ppcVar6 = (char **)__ZN10CVProfNode10GetSubNodeEPKciS1_i
                                        (ppcVar6,"CTFPlayer::Precache",0,"Player",4);
        *(char ***)(puVar3 + 0x1014) = ppcVar6;
        }
        puVar1 = (uint *)(*(int *)(puVar3 + 0x10a0) + 4 + (int)ppcVar6[0x1c] * 8);
        *puVar1 = *puVar1 | 4;
        __ZN10CVProfNode10EnterScopeEv(ppcVar6);
        puVar3[0x1010] = 0;
    }
    }*/
    PrecacheOFPlayer();
    PrecacheParticleSystem("achieved");
    BaseClass::Precache();

    //Code from Kay's branch, probably has to do with the following
    //decompiled code
    PreCacheKart();
	PreCacheMvM();
    /*if ((iVar2 != 0) && ((puVar3[0x1010] == '\0' || (*(int*)(puVar3 + 0x100c) != 0)))) {
        iVar2 = *(int*)(puVar3 + 0x19b8);
        iVar7 = _ThreadGetCurrentId();
        if (iVar2 == iVar7) {
            cVar4 = __ZN10CVProfNode9ExitScopeEv(*(undefined4*)(puVar3 + 0x1014));
            puVar8 = *(undefined**)(puVar3 + 0x1014);
            if (cVar4 != '\0') {
                puVar8 = *(undefined**)(puVar8 + 100);
                *(undefined**)(puVar3 + 0x1014) = puVar8;
            }
            *(bool*)(puVar3 + 0x1010) = puVar8 == puVar3 + 0x1018;
        }
    }*/
    //VPROF_SCOPE_END(); //Maybe, this could also be something else
}

//OFSTATUS: Nearly complete, only missing vprof(but that doesn't make any difference ingame)
void COFPlayer::PrecacheOFPlayer()
{
    /*uint* puVar1;
    int iVar2;
    undefined* puVar3;
    char cVar4;
    int iVar5;
    char** ppcVar6;
    int iVar7;
    undefined* puVar8;

    puVar3 = _g_VProfCurrentProfile;
    iVar2 = *(int*)(_g_VProfCurrentProfile + 0x100c);
    if (iVar2 != 0) {
        iVar7 = *(int*)(_g_VProfCurrentProfile + 0x19b8);
        iVar5 = _ThreadGetCurrentId();
        if (iVar7 == iVar5) {
            ppcVar6 = *(char***)(puVar3 + 0x1014);
            if (*ppcVar6 != "CTFPlayer::PrecacheTFPlayer") {
                ppcVar6 = (char**)__ZN10CVProfNode10GetSubNodeEPKciS1_i
                (ppcVar6, "CTFPlayer::PrecacheTFPlayer", 0, "Player", 4);
                *(char***)(puVar3 + 0x1014) = ppcVar6;
            }
            puVar1 = (uint*)(*(int*)(puVar3 + 0x10a0) + 4 + (int)ppcVar6[0x1c] * 8);
            *puVar1 = *puVar1 | 4;
            __ZN10CVProfNode10EnterScopeEv(ppcVar6);
            puVar3[0x1010] = 0;
        }
    }*/
    if (m_bOFPlayerNeedsPrecache) 
    {
        m_bOFPlayerNeedsPrecache = false;
        PrecachePlayerModels();
        PrecacheScriptSound("Player.Spawn");
        PrecacheScriptSound("TFPlayer.Pain");
        PrecacheScriptSound("TFPlayer.CritHit");
        PrecacheScriptSound("TFPlayer.CritHitMini");
        PrecacheScriptSound("TFPlayer.DoubleDonk");
        PrecacheScriptSound("TFPlayer.CritPain");
        PrecacheScriptSound("TFPlayer.CritDeath");
        PrecacheScriptSound("TFPlayer.FreezeCam");
        PrecacheScriptSound("TFPlayer.Drown");
        PrecacheScriptSound("TFPlayer.AttackerPain");
        PrecacheScriptSound("TFPlayer.SaveMe");
        PrecacheScriptSound("TFPlayer.CritBoostOn");
        PrecacheScriptSound("TFPlayer.CritBoostOff");
        PrecacheScriptSound("TFPlayer.Decapitated");
        PrecacheScriptSound("TFPlayer.ReCharged");
        PrecacheScriptSound("Camera.SnapShot");
        PrecacheScriptSound("TFPlayer.Dissolve");
        PrecacheScriptSound("Saxxy.TurnGold");
        PrecacheScriptSound("Icicle.TurnToIce");
        PrecacheScriptSound("Icicle.HitWorld");
        PrecacheScriptSound("Icicle.Melt");
        PrecacheScriptSound("DemoCharge.ChargeCritOn");
        PrecacheScriptSound("DemoCharge.ChargeCritOff");
        PrecacheScriptSound("DemoCharge.Charging");
        PrecacheScriptSound("TFPlayer.StunImpactRange");
        PrecacheScriptSound("TFPlayer.StunImpact");
        PrecacheScriptSound("Halloween.PlayerScream");
        PrecacheScriptSound("Halloween.PlayerEscapedUnderworld");
        PrecacheScriptSound("Game.YourTeamLost");
        PrecacheScriptSound("Game.YourTeamWon");
        PrecacheScriptSound("Game.SuddenDeath");
        PrecacheScriptSound("Game.Stalemate");
        PrecacheScriptSound("TV.Tune");
        PrecacheScriptSound("Announcer.AM_FirstBloodRandom");
        PrecacheScriptSound("Announcer.AM_CapEnabledRandom");
        PrecacheScriptSound("Announcer.AM_RoundStartRandom");
        PrecacheScriptSound("Announcer.AM_FirstBloodFast");
        PrecacheScriptSound("Announcer.AM_FirstBloodFinally");
        PrecacheScriptSound("Announcer.AM_FlawlessVictoryRandom");
        PrecacheScriptSound("Announcer.AM_FlawlessDefeatRandom");
        PrecacheScriptSound("Announcer.AM_FlawlessVictory01");
        PrecacheScriptSound("Announcer.AM_TeamScrambleRandom");
        PrecacheScriptSound("Taunt.MedicHeroic");
        PrecacheScriptSound("Taunt.GuitarRiff");
        PrecacheScriptSound("Powerup.ReducedDamage");
        PrecacheScriptSound("Tournament.PlayerReady");
        PrecacheScriptSound("Medic.AutoCallerAnnounce");
        PrecacheScriptSound("Player.FallDamageIndicator");
        PrecacheScriptSound("Player.FallDamageDealt");
        PrecacheParticleSystem("crit_text");
        PrecacheParticleSystem("miss_text");
        PrecacheParticleSystem("cig_smoke");
        PrecacheParticleSystem("speech_mediccall");
        PrecacheParticleSystem("speech_mediccall_auto");
        PrecacheParticleSystem("speech_taunt_all");
        PrecacheParticleSystem("speech_taunt_red");
        PrecacheParticleSystem("speech_taunt_blue");
        PrecacheParticleSystem("player_recent_teleport_blue");
        PrecacheParticleSystem("player_recent_teleport_red");
        PrecacheParticleSystem("particle_nemesis_red");
        PrecacheParticleSystem("particle_nemesis_blue");
        PrecacheParticleSystem("spy_start_disguise_red");
        PrecacheParticleSystem("spy_start_disguise_blue");
        PrecacheParticleSystem("burningplayer_red");
        PrecacheParticleSystem("burningplayer_blue");
        PrecacheParticleSystem("burningplayer_rainbow");
        PrecacheParticleSystem("blood_spray_red_01");
        PrecacheParticleSystem("blood_spray_red_01_far");
        PrecacheParticleSystem("pyrovision_blood");
        PrecacheParticleSystem("water_blood_impact_red_01");
        PrecacheParticleSystem("blood_impact_red_01");
        PrecacheParticleSystem("water_playerdive");
        PrecacheParticleSystem("water_playeremerge");
        PrecacheParticleSystem("healthgained_red");
        PrecacheParticleSystem("healthgained_blu");
        PrecacheParticleSystem("healthgained_red_large");
        PrecacheParticleSystem("healthgained_blu_large");
        PrecacheParticleSystem("healthgained_red_giant");
        PrecacheParticleSystem("healthgained_blu_giant");
        PrecacheParticleSystem("critgun_weaponmodel_red");
        PrecacheParticleSystem("critgun_weaponmodel_blu");
        PrecacheParticleSystem("overhealedplayer_red_pluses");
        PrecacheParticleSystem("overhealedplayer_blue_pluses");
        PrecacheParticleSystem("highfive_red");
        PrecacheParticleSystem("highfive_blue");
        PrecacheParticleSystem("god_rays");
        PrecacheParticleSystem("bl_killtaunt");
        PrecacheParticleSystem("birthday_player_circling");
        PrecacheParticleSystem("drg_fiery_death");
        PrecacheParticleSystem("drg_wrenchmotron_teleport");
        PrecacheParticleSystem("taunt_flip_land_red");
        PrecacheParticleSystem("taunt_flip_land_blue");
        PrecacheParticleSystem("tfc_sniper_mist");
        PrecacheParticleSystem("dxhr_sniper_rail_blue");
        PrecacheParticleSystem("dxhr_sniper_rail_red");
        PrecacheParticleSystem("tfc_sniper_distortion_trail");
        PrecacheParticleSystem("rps_rock_red");
        PrecacheParticleSystem("rps_paper_red");
        PrecacheParticleSystem("rps_scissors_red");
        PrecacheParticleSystem("rps_rock_red_win");
        PrecacheParticleSystem("rps_paper_red_win");
        PrecacheParticleSystem("rps_scissors_red_win");
        PrecacheParticleSystem("rps_rock_blue");
        PrecacheParticleSystem("rps_paper_blue");
        PrecacheParticleSystem("rps_scissors_blue");
        PrecacheParticleSystem("rps_rock_blue_win");
        PrecacheParticleSystem("rps_paper_blue_win");
        PrecacheParticleSystem("rps_scissors_blue_win");
        PrecacheParticleSystem("blood_decap");
        PrecacheParticleSystem("xms_icicle_idle");
        PrecacheParticleSystem("xms_icicle_impact");
        PrecacheParticleSystem("xms_icicle_impact_dryice");
        PrecacheParticleSystem("xms_icicle_melt");
        PrecacheParticleSystem("xms_ornament_glitter");
        PrecacheParticleSystem("xms_ornament_smash_blue");
        PrecacheParticleSystem("xms_ornament_smash_red");
        PrecacheParticleSystem("drg_pomson_muzzleflash");
        PrecacheParticleSystem("drg_pomson_impact");
        PrecacheParticleSystem("drg_pomson_impact_drain");
        PrecacheParticleSystem("dragons_fury_effect");
        PrecacheParticleSystem("dxhr_arm_muzzleflash");
        PrecacheModel("effects/beam001_red.vmt", true);
        PrecacheModel("effects/beam001_blu.vmt", true);
        PrecacheModel("effects/beam001_white.vmt", true);
        PrecacheModel("models/player/gibs/random_organ.mdl", true);
        PrecacheScriptSound("Weapon_Mantreads.Impact");
        PrecacheScriptSound("cleats_conc.StepLeft");
        PrecacheScriptSound("cleats_conc.StepRight");
        PrecacheScriptSound("cleats_dirt.StepLeft");
        PrecacheScriptSound("cleats_dirt.StepRight");
        PrecacheScriptSound("xmas.jingle");
        PrecacheScriptSound("xmas.jingle_higher");
        PrecacheScriptSound("PegLeg.StepRight");
        PrecacheParticleSystem("bombinomicon_burningdebris");
        PrecacheParticleSystem("bombinomicon_burningdebris_halloween");
        PrecacheParticleSystem("halloween_player_death_blue");
        PrecacheParticleSystem("halloween_player_death");
        PrecacheScriptSound("Bombinomicon.Explode");
        PrecacheScriptSound("Weapon_DRG_Wrench.Teleport");
        PrecacheScriptSound("Weapon_Pomson.Single");
        PrecacheScriptSound("Weapon_Pomson.SingleCrit");
        PrecacheScriptSound("Weapon_Pomson.Reload");
        PrecacheScriptSound("Weapon_Pomson.DrainedVictim");
        PrecacheScriptSound("BlastJump.Whistle");
        PrecacheScriptSound("Spy.TeaseVictim");
        PrecacheScriptSound("Demoman.CritDeath");
        PrecacheScriptSound("Heavy.Battlecry03");
        PrecacheModel("models/effects/resist_shield/resist_shield.mdl", true);
        PrecacheModel("models/props_mvm/mvm_revive_tombstone.mdl", true);
        PrecacheScriptSound("General.banana_slip");
        PrecacheScriptSound("Parachute_open");
        PrecacheScriptSound("Parachute_close");
        PrecacheModel("models/props_trainyard/bomb_eotl_blue.mdl", true);
        PrecacheModel("models/props_trainyard/bomb_eotl_red.mdl", true);
    }
    /*if ((iVar2 != 0) && ((puVar3[0x1010] == '\0' || (*(int*)(puVar3 + 0x100c) != 0)))) {
        iVar2 = *(int*)(puVar3 + 0x19b8);
        iVar7 = _ThreadGetCurrentId();
        if (iVar2 == iVar7) {
            cVar4 = __ZN10CVProfNode9ExitScopeEv(*(undefined4*)(puVar3 + 0x1014));
            puVar8 = *(undefined**)(puVar3 + 0x1014);
            if (cVar4 != '\0') {
                puVar8 = *(undefined**)(puVar8 + 100);
                *(undefined**)(puVar3 + 0x1014) = puVar8;
            }
            *(bool*)(puVar3 + 0x1010) = puVar8 == puVar3 + 0x1018;
        }
    }*/
    return;
}

//OFSTATUS: Incomplete
void COFPlayer::PrecachePlayerModels()
{
    //For testing
	for( int i = OF_CLASS_SCOUT; i < OF_CLASS_COUNT; i++ )
		PrecacheModel( GetPlayerClassData(i)->GetModelName() ); //TEMPORARY FOR TESTING ONLY

}


//OFSTATUS: Incomplete, only handles jointeam and in jointeam it only handles actual numbers.
bool COFPlayer::ClientCommand( const CCommand& args )
{

	if ( FStrEq( args[0], "jointeam" ) && args.ArgC() >= 2 )
	{
		if (args.ArgC() >= 2)
		{
			HandleCommand_JoinTeam(args[1]);
		}
		return true;
	}
	else if( FStrEq( args[0], "joinclass" ) && args.ArgC() >= 2 )
	{
		HandleCommand_JoinClass(args[1]);
		return true;
	}

	return BaseClass::ClientCommand( args );
}

// OFSTATUS: COMPLETE
// removed a bunch of mvm and bot bloat, also the param as i dont think its ever NOT 5
int COFPlayer::GetAutoTeam()
{
	COFTeam *pTeamRed = OFTeamMgr()->GetTeam(OF_TEAM_RED); // piVar7
	COFTeam *pTeamBlue = OFTeamMgr()->GetTeam(OF_TEAM_BLUE); // piVar6

	if (!pTeamRed || !pTeamBlue)
	{
		int iRedTeamCount = pTeamRed->GetNumPlayers();
		int iBlueTeamCount = pTeamBlue->GetNumPlayers();

		if (iRedTeamCount < iBlueTeamCount)
		{
			return OF_TEAM_RED;
		}

		if (iBlueTeamCount < iRedTeamCount)
		{
			return OF_TEAM_BLUE;
		}

		if (OFGameRules()->GetGameType() == OF_GAMETYPE_ESCORT || pTeamRed->GetRole() != 1)
		{
			return OF_TEAM_BLUE;
		}

		//pTeamRed->GetRole() != 1
		//if (param_1 == 5)
		//{
		int iRandom = RandomInt(0, 1);
		return iRandom ? OF_TEAM_RED : OF_TEAM_BLUE;
		//}
		/*
		else
		{
			// wont this crash the game..?
			return param_1;
		}
		*/
	}

	return TEAM_SPECTATOR;
}

// OFSTATUS: INCOMPLETE
void COFPlayer::HandleCommand_JoinTeam(const char* arg)
{
	if (OFGameRules()->State_Get() == GR_STATE_GAME_OVER) return;

	int iTeam = OF_TEAM_RED; // uVar17
	if (!stricmp(arg, "auto"))
	{
		iTeam = GetAutoTeam();
	}
	else if (!stricmp(arg, "spectate"))
	{
		iTeam = TEAM_SPECTATOR;
	}
	else if (!stricmp(arg, "spectatorarena"))
	{
		iTeam = TEAM_SPECTATOR;
	}
	else //if
	{
		for (int i = 0; i < ARRAYSIZE(g_aTeamNames); i++)
		{
			if (!stricmp(arg, g_aTeamNames[i]))
			{
				iTeam = i;
				break;
			}
		}
	}

	// uVar17 get replaced with iVar8 and gets run through a function filled with matchmaking stuff i presume so ignore that

	// OFTODO: there's some arena stuff going on here too, look into it later
	if (iTeam == TEAM_SPECTATOR) // || tf_arena_use_queue
	{
		if (!mp_allowspectators.GetBool() && !IsHLTV() && !IsReplay() && !OFGameRules()->IsInArenaMode())
		{
			// this is supposed to appear in the middle of the screen but thats looks ugly >:T
			ClientPrint(this, HUD_PRINTTALK, "#Cannot_Be_Spectator");
			return;
		}

		if (GetTeamNumber() != TEAM_UNASSIGNED && !IsDead())
		{
			CommitSuicide(false, true);
		}

		//m_bArenaSpectator = bVar18; // field_0x226c = 8812
		//DuelMiniGame_NotifyPlayerChangedTeam - ignore

		ChangeTeam(TEAM_SPECTATOR);

		//if (m_bArenaSpectator)
		//{
		//}

		if (mp_fadetoblack.GetBool())
		{
			color32_s color = { 0, 0, 0, 255 };
			UTIL_ScreenFade(this, color, 0.0, 0.0, FFADE_IN | FFADE_PURGE);
		}

		//if (OFGameRules()->IsInArenaMode() && m_bArenaSpectator)
		//{
		//	ShowViewPortPanel("class_blue");
		//}
	}
	else
	{
		if (iTeam == GetTeamNumber()) return;

		if (OFGameRules()->WouldChangeUnbalanceTeams(iTeam, GetTeamNumber()))
		{
			ShowViewPortPanel("team");
			return;
		}

		ChangeTeam(iTeam);

		// tf_arena_force_class.GetBool()

		ShowViewPortPanel((iTeam == OF_TEAM_RED) ? "class_red" : "class_blue");
	}

	/*
    int iTeam = atoi(arg);

    if (iTeam != GetTeamNumber())
    {
        ChangeTeam(iTeam);
        ForceRespawn();
    }
	*/
}

//OFSTATUS: Incomplete, all placeholder
void COFPlayer::HandleCommand_JoinClass(const char* arg)
{
	int iClass = UTIL_StringFieldToInt(arg, g_aRawPlayerClassNamesShort, OF_CLASS_COUNT);

    if( m_Class.m_iClass != iClass )
    {
		m_Class.m_iClass = iClass;
        ForceRespawn();
    }
}

//OFSTATUS: Incomplete, all placeholder
void COFPlayer::ChangeTeam(int iTeam)
{
	if (!GetGlobalOFTeam(iTeam))
	{
		Warning("COFPlayer::ChangeTeam(%d) - invalid team index.\n", iTeam);
		return;
	}

	// OFGameRules()->CanChangeTeam - just a check for more halloween stuff, ignore
	//if (OFGameRules() && OFGameRules()->CanChangeTeam(GetTeamNumber()))

	// incond 0x4d or 0x52
	// more halloween checks

	// ignore getteamassignmentoverride - just matchmaking stuff

	//RemoveAllOwnedEntitiesFromWorld(true);
	
	//CTFGameStats::Event_TeamChange

	//if (OFGameRules() && OFGameRules()->IsDefaultGameMode())

	int iPrevTeam = GetTeamNumber();

	BaseClass::ChangeTeam(iTeam);

	//if (OFGameRules() && OFGameRules()->IsInHighlanderMode())

	//RemoveNemesisRelationships();

	if (iTeam == TEAM_SPECTATOR)
	{
		StateEnter(TF_STATE_OBSERVER);
		RemoveAllWeapons();
		DestroyViewModels();
	}
	else if (iTeam == TEAM_UNASSIGNED)
	{
		StateEnter(TF_STATE_OBSERVER);
	}
	else
	{
		if ((iPrevTeam == OF_TEAM_RED || iPrevTeam == OF_TEAM_BLUE) && !IsDead())
		{
			CommitSuicide(false, true);
		}
		else if (iPrevTeam < FIRST_GAME_TEAM && IsDead() && mp_fadetoblack.GetBool())
		{
			color32_s color = { 0, 0, 0, 255 };
			UTIL_ScreenFade(this, color, 0.75, 0.0, FFADE_PURGE);
		}

		// OFTODO: gotta work on spy functions + figure out what field_0x1ac8 is
		/*
		for (int i = 1; i < gpGlobals->maxClients; i++)
		{
			COFPlayer *pPlayer = ToOFPlayer(UTIL_PlayerByIndex(i));

			if (pPlayer && pPlayer != this)
			{
				if (pPlayer->m_Shared.field_0x1ac8 == this || !pPlayer->m_Shared.field_0x1ac8 && pPlayer->m_Shared.GetDisguiseTeam() == iTeam)
				{
					pPlayer->m_Shared.FindDisguiseTarget();
				}
			}
		}
		*/
	}

	//m_Shared.RemoveAllCond();

	/*
    switch (iTeam)
    {
        case OF_TEAM_RED: m_nSkin = 0; break;
        case OF_TEAM_BLUE: m_nSkin = 1; break;
        default: m_nSkin = 0;
    }
    UpdateModel();
    BaseClass::ChangeTeam(iTeam);
	*/
}

// OFSTATUS: INCOMPLETE
// removed cond checks if your in hell
void COFPlayer::CommitSuicide(bool bExplode, bool bForce)
{
	// my guess is these lead to something in CTFPlayerShared judging by how high the hex values are
	//if ((*(int *)(this + 0x2114) == 0) || (*(int *)(this + 0x1a80) != 0)) return;

	if (!bForce && OFGameRules()->State_Get() == GR_STATE_TEAM_WIN)
	{
		if (GetTeamNumber() != OFGameRules()->GetWinningTeam()) return;
	}

	// 0xa9c = 2716 = m_bShowMatchSummary
	//if (OFGameRules()->m_bShowMatchSummary)

	//this->field_0x1fc5 = param_1;
	//this->field_0xb94 = 6;

	BaseClass::CommitSuicide(bExplode, bForce);
}

// OFSTATUS: COMPLETE
void COFPlayer::RemoveAllWeapons()
{
	ClearActiveWeapon();
	for (int i = 0; i < MAX_WEAPONS; i++)
	{
		CBaseCombatWeapon *pWeapon = m_hMyWeapons[i];
		if (pWeapon)
		{
			Weapon_Detach(pWeapon);
			UTIL_Remove(pWeapon);
		}
	}

	//m_Shared.RemoveDisguiseWeapon(); // OFTODO: UNCOMMENT ME LATER!

	// field_0xde0 leads to lots of econ stuff - ignore!
}

//OFSTATUS: INCOMPLETE
CBaseEntity *COFPlayer::EntSelectSpawnPoint()
{
	CBaseEntity *pLastPoint = g_pLastSpawnPoints[GetTeamNumber()]; // local_14

	// ignoring some matchsummary check here

	switch (GetTeamNumber())
	{
	case OF_TEAM_RED:
	case OF_TEAM_BLUE:

		if (SelectSpawnSpotByType("info_player_teamspawn", pLastPoint))
		{
			g_pLastSpawnPoints[GetTeamNumber()] = pLastPoint;
		}

		//field_0x2100 = pLastPoint; // unused

		break;
	default:
		return CBaseEntity::Instance(INDEXENT(0));
		break;
	}

	if (!pLastPoint)
	{
		Warning("COFPlayer::EntSelectSpawnPoint: no info_player_teamspawn on level!\n");
		return CBaseEntity::Instance(INDEXENT(0));
	}

	return pLastPoint;

    //return SelectSpawnSpotByType("info_player_teamspawn", nullptr);

    //return BaseClass::EntSelectSpawnPoint();
}

//OFSTATUS: INCOMPLETE
bool COFPlayer::SelectSpawnSpotByType(char* type, CBaseEntity* &pEntity)
{
	pEntity = gEntList.FindEntityByClassname(pEntity, type);

	if (!pEntity)
	{
		pEntity = gEntList.FindEntityByClassname(NULL, type);
	}

	//bool bCheck = !V_strcmp(type, "info_player_teamspawn");
	bool bDontIncludePlayers = false; //bVar4

	CBaseEntity *pFirstSpawn = pEntity;
	do
	{
		// 0x124 = 292 = m_spawnflags 
		if (pEntity && OFGameRules()->IsSpawnPointValid(pEntity, this, bDontIncludePlayers))
		{
			// there's a check here if the player's class can spawn at this point

			if (!V_strcmp(type, "info_player_teamspawn")) return true;
		}

		pEntity = gEntList.FindEntityByClassname(pEntity, type);

		if (pFirstSpawn == pEntity && !bDontIncludePlayers)
		{
			pEntity = gEntList.FindEntityByClassname(pEntity, type);
			bDontIncludePlayers = true;
		}
	} while (pFirstSpawn != pEntity);

	return false;

	/*
    CBaseEntity* pSpawn = gEntList.FindEntityByClassname(nullptr, type);

    while (pSpawn && pSpawn->GetTeamNumber() != GetTeamNumber())
    {
        pSpawn = gEntList.FindEntityByClassname(pSpawn, type);
    }

    if (pSpawn == nullptr)
    {
        Warning("Could not find %s for team %i, selecting non matching spawnpoint", type, GetTeamNumber());

        //Select a spawnpoint even though the team doesn't match, otherwise the game will crash
        pSpawn = gEntList.FindEntityByClassname(nullptr, type);
    }

    return pSpawn;
	*/
}

//OFSTATUS: Incomplete, needs CTFPlayerClassShared to get the model names
void COFPlayer::UpdateModel()
{
	//BaseClass::SetModel((COFPlayerClassShared*)(this[1].data + 0x688)->GetModelName());
	BaseClass::SetModel( m_Class.GetModelName() ); //TEMPORARY FOR TESTING ONLY
	SetCollisionBounds( BaseClass::GetPlayerMins(), BaseClass::GetPlayerMaxs() );
    //m_PlayerAnimState->OnNewModel(); //Crashes so disabled for now
}

COFWeaponBase *COFPlayer::GetActiveOFWeapon(void) const
{
	return dynamic_cast< COFWeaponBase* >(GetActiveWeapon());
}

extern ConVar friendlyfire;
extern ConVar sv_showimpacts;

// OFTODO: Move this to Shared Player
void COFPlayer::FireBullet( 
						   Vector vecSrc,	// shooting postion
						   const QAngle &shootAngles,  //shooting angle
						   float vecSpread, // spread vector
						   int iDamage, // base damage
						   int iBulletType, // ammo type
						   CBaseEntity *pevAttacker, // shooter
						   bool bDoEffects,	// create impact effect ?
						   float x,	// spread x factor
						   float y	// spread y factor
						   )
{
	float fCurrentDamage = iDamage;   // damage of the bullet at it's current trajectory
	float flCurrentDistance = 0.0;  //distance that the bullet has traveled so far

	Vector vecDirShooting, vecRight, vecUp;
	AngleVectors( shootAngles, &vecDirShooting, &vecRight, &vecUp );

	if ( !pevAttacker )
		pevAttacker = this;  // the default attacker is ourselves

	// add the spray 
	Vector vecDir = vecDirShooting +
		x * vecSpread * vecRight +
		y * vecSpread * vecUp;

	VectorNormalize( vecDir );

	float flMaxRange = 8000;

	Vector vecEnd = vecSrc + vecDir * flMaxRange; // max bullet range is 10000 units

	trace_t tr; // main enter bullet trace

	UTIL_TraceLine( vecSrc, vecEnd, MASK_SOLID|CONTENTS_DEBRIS|CONTENTS_HITBOX, this, COLLISION_GROUP_NONE, &tr );

		if ( tr.fraction == 1.0f )
			return; // we didn't hit anything, stop tracing shoot

	if ( sv_showimpacts.GetBool() )
	{
#ifdef CLIENT_DLL
		// draw red client impact markers
		debugoverlay->AddBoxOverlay( tr.endpos, Vector(-2,-2,-2), Vector(2,2,2), QAngle( 0, 0, 0), 255,0,0,127, 4 );

		if ( tr.m_pEnt && tr.m_pEnt->IsPlayer() )
		{
			C_BasePlayer *player = ToBasePlayer( tr.m_pEnt );
			player->DrawClientHitboxes( 4, true );
		}
#else
		// draw blue server impact markers
		NDebugOverlay::Box( tr.endpos, Vector(-2,-2,-2), Vector(2,2,2), 0,0,255,127, 4 );

		if ( tr.m_pEnt && tr.m_pEnt->IsPlayer() )
		{
			CBasePlayer *player = ToBasePlayer( tr.m_pEnt );
			player->DrawServerHitboxes( 4, true );
		}
#endif
	}

		//calculate the damage based on the distance the bullet travelled.
		flCurrentDistance += tr.fraction * flMaxRange;

		// damage get weaker of distance
		fCurrentDamage *= pow ( 0.85f, (flCurrentDistance / 500));

		int iDamageType = DMG_BULLET | DMG_NEVERGIB;

		if( bDoEffects )
		{
			// See if the bullet ended up underwater + started out of the water
			if ( enginetrace->GetPointContents( tr.endpos ) & (CONTENTS_WATER|CONTENTS_SLIME) )
			{	
				trace_t waterTrace;
				UTIL_TraceLine( vecSrc, tr.endpos, (MASK_SHOT|CONTENTS_WATER|CONTENTS_SLIME), this, COLLISION_GROUP_NONE, &waterTrace );

				if( waterTrace.allsolid != 1 )
				{
					CEffectData	data;
					data.m_vOrigin = waterTrace.endpos;
					data.m_vNormal = waterTrace.plane.normal;
					data.m_flScale = random->RandomFloat( 8, 12 );

					if ( waterTrace.contents & CONTENTS_SLIME )
					{
						data.m_fFlags |= FX_WATER_IN_SLIME;
					}

					DispatchEffect( "gunshotsplash", data );
				}
			}
			else
			{
				//Do Regular hit effects

				// Don't decal nodraw surfaces
				if ( !( tr.surface.flags & (SURF_SKY|SURF_NODRAW|SURF_HINT|SURF_SKIP) ) )
				{
					CBaseEntity *pEntity = tr.m_pEnt;
					if ( !( !friendlyfire.GetBool() && pEntity && pEntity->IsPlayer() && pEntity->GetTeamNumber() == GetTeamNumber() ) )
					{
						UTIL_ImpactTrace( &tr, iDamageType );
					}
				}
			}
		} // bDoEffects

		// add damage to entity that we hit

#ifdef GAME_DLL
		ClearMultiDamage();

		CTakeDamageInfo info( pevAttacker, pevAttacker, fCurrentDamage, iDamageType );
		CalculateBulletDamageForce( &info, iBulletType, vecDir, tr.endpos );
		tr.m_pEnt->DispatchTraceAttack( info, vecDir, &tr );

		TraceAttackToTriggers( info, tr.startpos, tr.endpos, vecDir );

		ApplyMultiDamage();
#endif
}