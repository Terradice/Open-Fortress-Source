// ========= Copyright Open Fortress Developers, CC-BY-NC-SA ============
// Purpose: Game logic manager
// Author(s): ficool2, Fenteale, Nopey
//

#include "cbase.h"
//#include "of_shareddefs.h"
#include "of_flag.h"
#include "of_gamerules.h"
#include "viewport_panel_names.h"
#include "gameeventdefs.h"
#include <KeyValues.h>
#include "ammodef.h"

#ifdef CLIENT_DLL
	#include "c_of_player.h"
#else
	#include "voice_gamemgr.h"
	#include "eventqueue.h"
	#include "player.h"
	#include "game.h"
	#include "items.h"
	#include "entitylist.h"
	#include "mapentities.h"
	#include "in_buttons.h"
	#include <ctype.h>
	#include "iscorer.h"
	#include "of_player.h"
	#include "gameinterface.h"
	#include "of_bot_temp.h"
	#include "of_team.h"
	#include "team_control_point_master.h"
	//#include "of_flag.h"
#endif

REGISTER_GAMERULES_CLASS( COFGameRules );

BEGIN_NETWORK_TABLE_NOBASE( COFGameRules, DT_OFGameRules )
#ifdef CLIENT_DLL
		RecvPropInt(RECVINFO(m_nGameType)),
#else
		SendPropInt(SENDINFO(m_nGameType), 4, SPROP_UNSIGNED),
#endif
END_NETWORK_TABLE()

LINK_ENTITY_TO_CLASS( tf_gamerules, COFGameRulesProxy );
IMPLEMENT_NETWORKCLASS_ALIASED( OFGameRulesProxy, DT_OFGameRulesProxy )

ConVar sv_showimpacts("sv_showimpacts", "0", FCVAR_REPLICATED, "Shows client (red) and server (blue) bullet impact point" );
ConVar tf_flag_caps_per_round("tf_flag_caps_per_round", "3", FCVAR_REPLICATED, "Number of captures per round on CTF maps. Set to 0 to disable.");
ConVar tf_flag_return_on_touch("tf_flag_return_on_touch", "0", FCVAR_REPLICATED, "If this is set, your flag must be at base in order to capture the enemy flag.Remote friendly flags return to your base instantly when you touch them.");

// OFSTATUS: COMPLETE
static const char *s_PreserveEnts[] =
{
	"worldspawn",
	"tf_gamerules",
	"tf_player_manager",
	"tf_team",
	"team_manager",
	"tf_objective_resource",
	"keyframe_rope",
	"move_rope",
	"tf_",
	"tf_logic_training",
	"tf_logic_training_mode",
	"tf_powerup_bottle",
	"tf_mann_vs_machine_stats",
	"tf_wearable",
	"tf_wearable_demoshield",
	"tf_wearable_robot_arm",
	"tf_wearable_vm",
	"tf_logic_bonusround",
	"vote_controller",
	"monster_resource",
	"tf_logic_medieval",
	"tf_logic_cp_timer",
	"tf_logic_tower_defense",
	"tf_logic_mann_vs_machine",
	"func_upgradestation",
	"entity_rocket",
	"entity_carrier",
	"entity_sign",
	"entity_saucer",
	"tf_halloween_gift_pickup",
	"tf_logic_competitive",
	"tf_wearable_razorback",
	"entity_soldier_statue",
	"", // END Marker
};

#ifdef CLIENT_DLL
	void RecvProxy_OFGameRules( const RecvProp *pProp, void **pOut, void *pData, int objectID )
	{
		COFGameRules *pRules = OFGameRules();
		Assert( pRules );
		*pOut = pRules;
	}

	BEGIN_RECV_TABLE( COFGameRulesProxy, DT_OFGameRulesProxy )
		RecvPropDataTable( "tf_gamerules_data", 0, 0, &REFERENCE_RECV_TABLE( DT_OFGameRules ), RecvProxy_OFGameRules )
	END_RECV_TABLE()
#else
	void* SendProxy_OFGameRules( const SendProp *pProp, const void *pStructBase, const void *pData, CSendProxyRecipients *pRecipients, int objectID )
	{
		COFGameRules *pRules = OFGameRules();
		Assert( pRules );
		return pRules;
	}

	BEGIN_SEND_TABLE( COFGameRulesProxy, DT_OFGameRulesProxy )
		SendPropDataTable( "tf_gamerules_data", 0, &REFERENCE_SEND_TABLE( DT_OFGameRules ), SendProxy_OFGameRules )
	END_SEND_TABLE()
#endif

// Called by world.cpp, is a NOP in both SDK and TF2.
// OFSTATUS: COMPLETE.
void InitBodyQue()
{
}


// OFSTATUS: INCOMPLETE	
COFGameRules::COFGameRules()
{
#ifdef GAME_DLL
	OFTeamMgr()->Init();

	ResetMapTime();

	// Create the team managers
	/*
	for ( int i = 0; i < ARRAYSIZE( g_aTeamNames ); i++ )
	{
		CTeam *pTeam = static_cast<CTeam*>(CreateEntityByName( "team_manager" ));
		pTeam->Init( g_aTeamNames[i], i );
	
		g_Teams.AddToTail( pTeam );
	}
	*/

	ListenForGameEvent("teamplay_point_captured");
	ListenForGameEvent("teamplay_capture_blocked");
	ListenForGameEvent("teamplay_round_win");
	ListenForGameEvent("teamplay_flag_event");
	ListenForGameEvent("teamplay_round_start");
	ListenForGameEvent("player_escort_score");
	ListenForGameEvent("player_disconnect");
	ListenForGameEvent("teamplay_setup_finished");
	ListenForGameEvent("recalculate_truce");
#else
	ListenForGameEvent("game_newmap");
	ListenForGameEvent("overtime_nag");
	ListenForGameEvent("recalculate_holidays");
#endif
}
	
// OFSTATUS: INCOMPLETE	
COFGameRules::~COFGameRules( void )
{
#ifdef GAME_DLL
	OFTeamMgr()->Shutdown();
#endif
}

#ifdef GAME_DLL
// OFSTATUS: COMPLETE
// cut mvm and boss precaching
void COFGameRules::Precache(void)
{
	CTeamplayRules::Precache();
	COFPlayer::m_bOFPlayerNeedsPrecache = true;
}

// OFSTATUS: INCOMPLETE
void COFGameRules::CreateStandardEntities( void )
{
	m_hOFGameRulesProxy = dynamic_cast< COFGameRulesProxy* >(CBaseEntity::Create("tf_gamerules", vec3_origin, vec3_angle));
}

// OFSTATUS: INCOMPLETE
void COFGameRules::Think( void )
{
	//BaseClass::Think();
}
#endif

// OFSTATUS: COMPLETE
float COFGameRules::GetRespawnTimeScalar(int iTeam)
{
	// (field_0x964 + 2) is PVE mode (MvM)
	//if (*(char *)((int)&this->field_0x964 + 2) != '\0') 
	//{
	//	return (float10)1;
	//}

	//fVar1 = (float10)CTeamplayRoundBasedRules::GetRespawnTimeScalar((CTeamplayRoundBasedRules *)this, param_1);
	//return fVar1;
	return BaseClass::GetRespawnTimeScalar(iTeam);
}

// OFSTATUS: COMPLETE
float COFGameRules::GetRespawnWaveMaxLength(int iTeam, bool bScaleWithNumPlayers)
{
	//fVar1 = (float10)CTeamplayRoundBasedRules::GetRespawnWaveMaxLength
	//	((CTeamplayRoundBasedRules *)this, param_1,
	//	(bool)(*(char *)((int)&this->field_0x964 + 2) == '\0' & param_2));
	//fVar2 = (float)fVar1;

	return BaseClass::GetRespawnWaveMaxLength(iTeam, bScaleWithNumPlayers);

	// more mvm stuff
	//this_00 = (CTFRobotDestructionLogic *)CTFRobotDestructionLogic::GetRobotDestructionLogic();
	//if (this_00 != (CTFRobotDestructionLogic *)0x0)
	//{
	//	fVar1 = (float10)CTFRobotDestructionLogic::GetRespawnScaleForTeam(this_00, param_1);
	//	fVar2 = fVar2 * (1.0 - (float)fVar1);
	//}
	//return (float10)fVar2;
}

// OFSTATUS: COMPLETE
bool COFGameRules::FlagsMayBeCapped()
{
	if ((State_Get() == GR_STATE_PREROUND) || (State_Get() == GR_STATE_TEAM_WIN))
		return false;

	return true;
}

#ifdef GAME_DLL
// OFSTATUS: COMPLETE
void COFGameRules::RemoveAllProjectiles()
{
	// OFTODO: we need to implement projectiles before we can uncomment this
	//for (int i = 0; i < IBaseProjectileAutoList::AutoList().Count(); i++)
	//{
	//	UTIL_Remove(static_cast<CBaseProjectile*>(IBaseProjectileAutoList::AutoList()[i]));
	//}
}

// OFSTATUS: INCOMPLETE
// OFTODO: gotta implement objects first
void COFGameRules::RemoveAllBuildings(bool param_1)
{
	/*
	for (int i = 0; i < IBaseObjectAutoList::AutoList().Count(); i++)
	{
		CBaseObject *pObj = static_cast<CBaseObject*>(IBaseObjectAutoList::AutoList()[i]);

		if (!(pObj + 0x178))
		{
			IGameEvent *event = gameeventmanager->CreateEvent("object_removed");
			if (event)
			{
				iVar7 = (**(code **)(*piVar1 + 0x51c))(piVar1);
				pcVar3 = *(code **)(*piVar6 + 0x2c);
				uVar8 = 0xffffffff;
				if (iVar7 != 0) {
					uVar8 = (**(code **)(**(int **)PTR__engine_00e34074 + 0x3c))
						(*(int **)PTR__engine_00e34074, *(undefined4 *)(iVar7 + 0x20));
				}
				(*pcVar3)(piVar6, "userid", uVar8);
				pcVar3 = *(code **)(*piVar6 + 0x2c);
				uVar8 = (**(code **)(*(int *)(iVar2 + -0x864) + 0x580))(piVar1);
				(*pcVar3)(piVar6, "objecttype", uVar8);
				iVar7 = 0;
				if (*(int *)(iVar2 + -0x844) != 0) {
					iVar7 = (int)*(short *)(*(int *)(iVar2 + -0x844) + 6);
				}
				(**(code **)(*piVar6 + 0x2c))(piVar6, "index", iVar7);

				gameeventmanager->FireEvent(event);
			}

			if (param_1)
			{
				//(**(code **)(*piVar1 + 0x568))(piVar1);
			}
			else
			{
				// why do we set something nonsolid when we're about to destroy it..
				pObj->SetSolid(SOLID_NONE);
				UTIL_Remove(pObj);
			}
		}
	}
	*/
}

// OFSTATUS: INCOMPLETE
// gotta implement objects
void COFGameRules::RemoveAllSentriesAmmo()
{
	/*
	for (int i = 0; i < IBaseObjectAutoList::AutoList().Count(); i++)
	{
		CBaseObject *pObj = static_cast<CBaseObject*>(IBaseObjectAutoList::AutoList()[i]);
		// check if the object is a sentry, if so remove the ammo
		//iVar2 = (**(code **)(*this + 0x580))(this);
		//if (iVar2 == 2)
		//{
		//	CObjectSentrygun::RemoveAllAmmo((CObjectSentrygun *)this);
		//}
	}
	*/
}

// OFSTATUS: COMPLETE
// implement objects :lilacstate:
void COFGameRules::RemoveAllProjectilesAndBuildings(bool param_1)
{
	RemoveAllProjectiles();
	RemoveAllBuildings(param_1);
}

// OFSTATUS: COMPLETE
bool COFGameRules::CanChangelevelBecauseOfTimeLimit()
{
	CTeamControlPointMaster *pMaster = g_hControlPointMasters.Count() ? g_hControlPointMasters[0] : NULL;
	if (pMaster && pMaster->PlayingMiniRounds() && pMaster->ShouldPlayAllControlPointRounds() && !m_bForceMapReset && 0 < pMaster->NumPlayableControlPointRounds())
	{
		return false;
	}
	return true;
}

// OFSTATUS: COMPLETE
bool COFGameRules::CanGoToStalemate()
{
	if (m_nGameType == TF_GAMETYPE_CTF)
	{
		for (int i = 0; i < ICaptureFlagAutoList::AutoList().Count(); i++)
		{
			CCaptureFlag *pFlag = static_cast<CCaptureFlag*>(ICaptureFlagAutoList::AutoList()[i]);
			if (pFlag->IsDropped() || pFlag->IsStolen())
			{
				return false;
			}
		}

		if (CheckCapsPerRound())
		{
			return false;
		}
	}
	return true;
}

// OFSTATUS: COMPLETE
// we dont really care for passtime so this can be trimmed down to just:
bool COFGameRules::CheckCapsPerRound()
{
	return SetCtfWinningTeam();
}

// OFSTATUS: COMPLETE
bool COFGameRules::SetCtfWinningTeam()
{
	if (0 < tf_flag_caps_per_round.GetInt())
	{
		int iTeamCount = OFTeamMgr()->GetTeamCount();
		int iWinningCaps = -1;
		COFTeam *iWinningTeam = NULL;

		for (int i = FIRST_GAME_TEAM; i < iTeamCount; i++)
		{
			COFTeam *pTeam = GetGlobalOFTeam(i);
			if (pTeam)
			{
				if (iWinningCaps < pTeam->GetFlagCaptures() && tf_flag_caps_per_round.GetInt() <= pTeam->GetFlagCaptures())
				{
					iWinningCaps = pTeam->GetFlagCaptures();
					iWinningTeam = pTeam;
				}
			}
		}

		if ((iWinningTeam != NULL) && (iWinningCaps != -1))
		{
			SetWinningTeam(iWinningTeam->GetTeamNumber(), WINREASON_FLAG_CAPTURE_LIMIT, true, false, false, false);
			return true;
		}
	}
	return false;
}

bool COFGameRules::CanFlagBeCaptured(COFPlayer *pPlayer)
{
	/*
	undefined *puVar1;
	char cVar2;
	int iVar3;
	int iVar4;
	int iVar5;
	CCaptureFlag *this_00;

	int iFlagCount = 0;

	if (!pPlayer)
	{
		if (tf_flag_return_on_touch.GetInt() == 0)
		{
			if (this[0x961] == (CTFGameRules)0x0)
			{
				return true;
			}
			iVar3 = *(int *)(PTR_m_ICaptureFlagAutoListAutoList_00e344f8 + 0xc);
		}
		else
		{
			iVar3 = *(int *)(PTR_m_ICaptureFlagAutoListAutoList_00e344f8 + 0xc);
		}
		iVar5 = 0;
		if (0 < iVar3)
		{
			do
			{
				puVar1 = PTR_m_ICaptureFlagAutoListAutoList_00e344f8;
				iVar3 = *(int *)(*(int *)PTR_m_ICaptureFlagAutoListAutoList_00e344f8 + iVar5 * 4);
				if (*(int *)(iVar3 + 0x24) == 0)
				{
					this_00 = (CCaptureFlag *)(iVar3 + -0x64c);
					cVar2 = CCaptureFlag::IsDisabled(this_00);
					if (cVar2 == '\0')
					{
						iVar3 = CBaseEntity::GetTeamNumber(param_1);
						iVar4 = CBaseEntity::GetTeamNumber((CBaseEntity *)this_00);
						if ((iVar3 == iVar4) && (cVar2 = CCaptureFlag::IsHome(this_00), cVar2 == '\0'))
						{
							return false;
						}
					}
				}
				iVar5 = iVar5 + 1;
			} while (iVar5 < *(int *)(puVar1 + 0xc));
		}
	}
	*/
	return true;
}

#endif // GAME_DLL

// OFSTATUS: INCOMPLETE (theres some kind of holiday calc here)
void COFGameRules::GoToIntermission( void )
{
#ifndef CLIENT_DLL
#endif
}

//OFSTATUS: INCOMPLETE
int COFGameRules::PlayerRelationship( CBaseEntity *pPlayer, CBaseEntity *pTarget )
{
	return 0;
}

// OFSTATUS: INCOMPLETE
void COFGameRules::ClientDisconnected( edict_t *pClient )
{
#ifndef CLIENT_DLL
	CBasePlayer *pPlayer = (CBasePlayer *)CBaseEntity::Instance( pClient );
	if ( pPlayer )
	{
		// Remove the player from his team
		if ( pPlayer->GetTeam() )
		{
			pPlayer->GetTeam()->RemovePlayer( pPlayer );
		}
	}

	BaseClass::ClientDisconnected( pClient );

#endif
}


// OFSTATUS: INCOMPLETE ( this is a big function! )
void COFGameRules::DeathNotice( CBasePlayer *pVictim, const CTakeDamageInfo &info )
{
#ifndef CLIENT_DLL

	// Work out what killed the player, and send a message to all clients about it
	const char *killer_weapon_name = "world";		// by default, the player is killed by the world
	int killer_ID = 0;

	// Find the killer & the scorer
	CBaseEntity *pInflictor = info.GetInflictor();
	CBaseEntity *pKiller = info.GetAttacker();
	CBasePlayer *pScorer = GetDeathScorer( pKiller, pInflictor );

	// Custom kill type?
	if ( info.GetDamageCustom() )
	{
		killer_weapon_name = GetDamageCustomString( info );
		if ( pScorer )
		{
			killer_ID = pScorer->GetUserID();
		}
	}
	else
	{
		// Is the killer a client?
		if ( pScorer )
		{
			killer_ID = pScorer->GetUserID();
			
			if ( pInflictor )
			{
				if ( pInflictor == pScorer )
				{
					// If the inflictor is the killer,  then it must be their current weapon doing the damage
					if ( pScorer->GetActiveWeapon() )
					{
						killer_weapon_name = pScorer->GetActiveWeapon()->GetClassname();
					}
				}
				else
				{
					killer_weapon_name = pInflictor->GetClassname();  // it's just that easy
				}
			}
		}
		else
		{
			killer_weapon_name = pInflictor->GetClassname();
		}
	}

	IGameEvent *event = gameeventmanager->CreateEvent( "player_death" );
	if( event )
	{
		event->SetInt("userid", pVictim->GetUserID() );
		event->SetInt("attacker", killer_ID );
		event->SetString("weapon", killer_weapon_name );
		event->SetInt( "priority", 7 );
		gameeventmanager->FireEvent( event );
	}
#endif
}

// OFSTATUS: COMPLETE
const char *COFGameRules::GetGameDescription( void )
{ 
	return "Open Fortress";
} 

// OFSTATUS: INCOMPLETE 
bool COFGameRules::IsConnectedUserInfoChangeAllowed( CBasePlayer *pPlayer )
{
	/*
	if ( pPlayer && ( pPlayer->??? != '\0' ) ) {
		if ( (int*) pPlayer[0x10c] ){
			return true;
		}
		uint team = GetTeamNumber(pPlayer);
		// 2 and 3 are BLU and RED, aren't they?
		if ( team!=2 && team!=3 ) {
			return true;
		}
		if ( gpGlobals->absoluteframetime - (float*)(pPlayer + 0x21a8) < 2) {
			return true;
		}
		// COFPlayerShared::ShouldSuppressPrediction
		bool suppressed = (pPlayer + 0x19b8)->ShouldSuppressPrediction();
		int clientIndex = 0;
		if (*(int *)(pPlayer + 0x20) != 0) {
			clientIndex = (int)*(short *)(*(int *)(pPlayer + 0x20) + 6);
		}
		const char *client_predict = engine->GetClientConVarValue("cl_predict");
		// some bs xor stuff?
		// if (((!suppressed) != ((byte *)pPlayer)[0xade]) && ((V_atoi(client_predict) != 0) == (bool)(suppressed ^ 1U))) {
		// potentially equivalent, idk man ask the science team:
		if ((suppressed != (!((byte *)pPlayer)[0xade])) && ((!V_atoi(client_predict))==suppressed)) {
			return true;
		}
	}
	return false;
	*/
	return true;
}

// OFSTATUS: COMPLETE
// cut halloween and matchmaking
void COFGameRules::LevelInitPostEntity()
{
	CTeamplayRoundBasedRules::LevelInitPostEntity();

	#ifdef GAME_DLL

	m_hOFGameRulesProxy = dynamic_cast<COFGameRulesProxy*>(gEntList.FindEntityByClassname(NULL, "tf_gamerules"));

	#endif
}

// OFSTATUS: NEEDS LABELS
bool COFGameRules::ShouldCollide( int collisionGroup0, int collisionGroup1 )
{
	if ( collisionGroup0 > collisionGroup1 )
	{
		V_swap( collisionGroup0, collisionGroup1 );
	}

	bool var1 = false;
	switch ( collisionGroup0 )
	{
		case COLLISION_GROUP_PLAYER_MOVEMENT:
			if ( collisionGroup1 == COLLISION_GROUP_WEAPON || collisionGroup1 == COLLISION_GROUP_PROJECTILE 
				|| collisionGroup1 == OF_COLLISION_GROUP_ROCKETS || collisionGroup1 == OF_COLLISION_GROUP_PROJECTILE )
				return false;		
			if ( collisionGroup1 == OF_COLLISION_GROUP_UNKNOWN20 )
				return false;
			break;		
		case COLLISION_GROUP_PLAYER:
			if ( collisionGroup1 == OF_COLLISION_GROUP_ROCKETS || collisionGroup1 == OF_COLLISION_GROUP_PROJECTILE )
				return true;
			if ( collisionGroup1 == OF_COLLISION_GROUP_UNKNOWN20 )
				return false;
			var1 = true;
			break;
		case COLLISION_GROUP_WEAPON:
		case OF_COLLISION_GROUP_UNKNOWN20:
		case COLLISION_GROUP_PROJECTILE:
			if ( collisionGroup1 == OF_COLLISION_GROUP_ROCKETS )
				return true;
			break;
		case OF_COLLISION_GROUP_ROCKETS:
		case OF_COLLISION_GROUP_PROJECTILE:
			break;
		default:
			var1 = true;
			break;
	}
	
	if ( !var1 && collisionGroup1 == OF_COLLISION_GROUP_PROJECTILE )
		return false; 
	
	if ( collisionGroup1 != OF_COLLISION_GROUP_UNKNOWN25 )
	{
		switch ( collisionGroup0 )
		{
			case OF_COLLISION_GROUP_UNKNOWN20:
				if ( collisionGroup1 == OF_COLLISION_GROUP_UNKNOWN20 )
					return false;
				return BaseClass::ShouldCollide( collisionGroup0, collisionGroup1 );
			case COLLISION_GROUP_PLAYER_MOVEMENT:
				if ( collisionGroup1 == OF_COLLISION_GROUP_UNKNOWN23 )
					return false;			
				break;
			case COLLISION_GROUP_PLAYER:
				if ( collisionGroup1 == OF_COLLISION_GROUP_UNKNOWN23 )
					return false;				
				break;
			default:
				return BaseClass::ShouldCollide( collisionGroup0, collisionGroup1 );
		}
		
		if ( collisionGroup1 != OF_COLLISION_GROUP_PUMPKIN_BOMB )
			return BaseClass::ShouldCollide( collisionGroup0, collisionGroup1 );
		
		return false;
	}
	
	return ( collisionGroup0 == COLLISION_GROUP_PLAYER || collisionGroup0 == COLLISION_GROUP_PLAYER_MOVEMENT );
}

// OFSTATUS: INCOMPLETE 
bool COFGameRules::ClientCommand( CBaseEntity *pEdict, const CCommand &args )
{
#ifndef CLIENT_DLL
	if ( BaseClass::ClientCommand( pEdict, args ) )
		return true;

	COFPlayer *pPlayer = (COFPlayer *) pEdict;

	if ( pPlayer->ClientCommand( args ) )
		return true;
#endif

	return false;
}

//OFSTATUS: Incomplete, and low priority
bool COFGameRules::IsPlayingSpecialDeliveryMode( void ) {
	return false;
}

ConVar ammo_max( "ammo_max", "5000", FCVAR_GAMEDLL | FCVAR_REPLICATED );

#ifdef GAME_DLL
#include "of_team.h"
// OFSTATUS: Incomplete
void COFGameRules::TeamPlayerCountChanged(COFTeam *pTeam)
{

}
#endif

// OFSTATUS: COMPLETE
CAmmoDef *GetAmmoDef()
{
	static CAmmoDef def;
	static bool bInitted = false;
	
	if ( !bInitted )
	{
		bInitted = true;
	
		int iAmmoIndex = AMMONAME_FIRST;
		do
		{
			def.AddAmmoType( g_aAmmoNames[iAmmoIndex], DMG_BULLET | DMG_NOCLOSEDISTANCEMOD | DMG_USEDISTANCEMOD, 1,	0, 0, ammo_max.GetInt(), 2400.0, 10, 14, 8 );
			iAmmoIndex++;
		}
		while ( iAmmoIndex != AMMONAME_LAST );
	}

	return &def;
}

#ifdef GAME_DLL
// OFSTATUS: INCOMPLETE (move this to of_bot_temp?) 
void Bot_f()
{
	// Look at -count.
	int count = 1;
	count = clamp( count, 1, 16 );

	// what is TEAM_COMBINE? we're not playing hl2 here..
	// oh well. OFTODO: What team should this be?
	int iTeam = 0; // TEAM_COMBINE;

	// Look at -frozen.
	bool bFrozen = false;

	// Ok, spawn all the bots.
	while ( --count >= 0 )
	{
		BotPutInServer( bFrozen, iTeam );
	}
}

ConCommand cc_Bot( "bot", Bot_f, "Add a bot.", FCVAR_CHEAT );

//OFTODO: This is just copy pasted from sdk_gamerules.cpp
// --------------------------------------------------------------------------------------------------- //
// Voice helper
// --------------------------------------------------------------------------------------------------- //

class CVoiceGameMgrHelper : public IVoiceGameMgrHelper
{
public:
	virtual bool		CanPlayerHearPlayer( CBasePlayer *pListener, CBasePlayer *pTalker, bool &bProximity )
	{
		// Dead players can only be heard by other dead team mates
		if ( pTalker->IsAlive() == false )
		{
			if ( pListener->IsAlive() == false )
				return ( pListener->InSameTeam( pTalker ) );

			return false;
		}

		return ( pListener->InSameTeam( pTalker ) );
	}
};
CVoiceGameMgrHelper g_VoiceGameMgrHelper;
IVoiceGameMgrHelper *g_pVoiceGameMgrHelper = &g_VoiceGameMgrHelper;

#endif