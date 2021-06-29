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
	#include "c_of_objectiveresource.h"
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
	#include "hltvdirector.h"
	#include "vote_controller.h"
	#include "of_objectiveresource.h"
	#include "team_train_watcher.h"
	#include "entity_ofstart.h"
	//#include "of_flag.h"
#endif

REGISTER_GAMERULES_CLASS( COFGameRules );

BEGIN_NETWORK_TABLE_NOBASE( COFGameRules, DT_OFGameRules )
#ifdef CLIENT_DLL
	RecvPropInt(RECVINFO(m_nHudType)),
	RecvPropInt(RECVINFO(m_nGameType)),
	RecvPropBool(RECVINFO(m_bPlayingKoth)),
	RecvPropBool(RECVINFO(m_bPlayingMedieval)),
	RecvPropBool(RECVINFO(m_bPlayingSpecialDeliveryMode)),
	RecvPropFloat(RECVINFO(m_flCapturePointEnableTime)),
#else
	SendPropInt(SENDINFO(m_nHudType), 3, SPROP_UNSIGNED),
	SendPropInt(SENDINFO(m_nGameType), 4, SPROP_UNSIGNED),
	SendPropBool(SENDINFO(m_bPlayingKoth)),
	SendPropBool(SENDINFO(m_bPlayingMedieval)),
	SendPropBool(SENDINFO(m_bPlayingSpecialDeliveryMode)),
	SendPropFloat(SENDINFO(m_flCapturePointEnableTime)),
#endif
END_NETWORK_TABLE()

LINK_ENTITY_TO_CLASS( tf_gamerules, COFGameRulesProxy );
IMPLEMENT_NETWORKCLASS_ALIASED( OFGameRulesProxy, DT_OFGameRulesProxy )

ConVar sv_showimpacts("sv_showimpacts", "0", FCVAR_REPLICATED, "Shows client (red) and server (blue) bullet impact point" );
ConVar tf_caplinear("tf_caplinear", "1", FCVAR_REPLICATED, "");
extern ConVar tf_flag_caps_per_round;
extern ConVar tf_flag_return_on_touch;

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

	BEGIN_SEND_TABLE(COFGameRulesProxy, DT_OFGameRulesProxy)
		SendPropDataTable("tf_gamerules_data", 0, &REFERENCE_SEND_TABLE(DT_OFGameRules), SendProxy_OFGameRules)
	END_SEND_TABLE()
#endif

#ifdef GAME_DLL
BEGIN_DATADESC(COFGameRulesProxy)
	DEFINE_KEYFIELD(m_nHudType, FIELD_INTEGER, "hud_type"),
	DEFINE_KEYFIELD(m_bOvertimeAllowedForCTF, FIELD_BOOLEAN, "ctf_overtime"),

	DEFINE_INPUTFUNC(FIELD_VOID, "SetRedTeamRespawnWaveTime", SetRedTeamRespawnWaveTime),
	// there's a lot more inputs here but i havent done them yet since, there's a lot :v - cherry

	DEFINE_OUTPUT(m_OnWonByTeam1, "OnWonByTeam1"),
	DEFINE_OUTPUT(m_OnWonByTeam2, "OnWonByTeam2"),
END_DATADESC();

// OFSTATUS: INCOMPLETE
void COFGameRulesProxy::SetRedTeamRespawnWaveTime(inputdata_t &inputdata)
{

}

COFGameRulesProxy::COFGameRulesProxy()
{
	m_nHudType = OF_HUDTYPE_UNDEFINDED;
	m_bOvertimeAllowedForCTF = false;
}

// OFSTATUS: COMPLETE
void COFGameRulesProxy::Activate()
{
	OFGameRules()->Activate();
	OFGameRules()->SetHudType(m_nHudType);

	OFGameRules()->SetOvertimeAllowedForCTF(m_bOvertimeAllowedForCTF);

	ListenForGameEvent("teamplay_round_win");

	BaseClass::Activate();
}
#endif

// OFSTATUS: COMPLETE
void COFGameRulesProxy::FireGameEvent(IGameEvent *event)
{
#ifdef GAME_DLL
	if (Q_strcmp(event->GetName(), "teamplay_round_win"))
	{
		switch (event->GetInt())
		{
		case OF_TEAM_RED:
			m_OnWonByTeam1.FireOutput(this, this);
			break;
		case OF_TEAM_BLUE:
			m_OnWonByTeam2.FireOutput(this, this);
			break;
		}
	}
#endif
}

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

#ifdef GAME_DLL
	m_iPreviousRoundWinnerTeam = TEAM_UNASSIGNED;
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
void COFGameRules::Think( void )
{
	if (!g_fGameOver)
	{
		if (m_flNextPeriodicThink < gpGlobals->curtime)
		{
			if (State_Get() != GR_STATE_TEAM_WIN && State_Get() != GR_STATE_BONUS && State_Get() != GR_STATE_GAME_OVER && !IsInWaitingForPlayers())
			{
				if (CheckCapsPerRound()) return;
			}
		}
	}

	BaseClass::Think();
}

// OFSTATUS: INCOMPLETE
// i did my best in dissecting the parts that we actually need
// there's stuff commented out as we havent implemented it yet - cherry
void COFGameRules::Activate()
{
	m_nGameType = OF_GAMETYPE_UNDEFINDED;

	m_bPlayingKoth = false; // field_0x960
	m_bPlayingMedieval = false; // field_0x963
	//m_bPlayingHybrid_CTF_CP = false // field_0x964 - this exists????
	m_bPlayingSpecialDeliveryMode = false; // field_0x965

	/*
	// Arena
	CArenaLogic *pLogicArenaEnt = dynamic_cast<CArenaLogic*>(gEntList.FindEntityByClassname(NULL,"tf_logic_arena"));
	if (pLogicArenaEnt)
	{
		field_0x67c = pLogicArenaEnt;
		m_nGameType = TF_GAMETYPE_ARENA;

		Msg("Executing server arena config file\n");
		engine->ServerCommand("exec config_arena.cfg\n");
	}
	*/

	// Control Points
	if (g_hControlPointMasters.Count() && m_nGameType != OF_GAMETYPE_ARENA)
	{
		m_nGameType = OF_GAMETYPE_CP;
	}

	// Special Delivery
	bool bFlags = (ICaptureFlagAutoList::AutoList().Count() > 0);
	if (StringHasPrefix(STRING(gpGlobals->mapname), "sd_"))
	{
		m_bPlayingSpecialDeliveryMode = true;
	}

	// Capture the Flag
	else if (bFlags)
	{
		m_nGameType = OF_GAMETYPE_CTF;
	}

	// Payload
	CTeamTrainWatcher *pTrainWatcher = dynamic_cast<CTeamTrainWatcher*>(gEntList.FindEntityByClassname(NULL, "team_train_watcher"));
	if (pTrainWatcher)
	{
		m_nGameType = OF_GAMETYPE_ESCORT;
	}

	/*
	// Koth
	CKothLogic *pKothLogic = dynamic_cast<CKothLogic*>(gEntList.FindEntityByClassname(NULL, "tf_logic_koth"));
	if (pKothLogic)
	{
		m_bPlayingKoth = true;
	}
	*/

	// Medieval
	/*
	CMedievalLogic *pMedievalLogic = dynamic_cast<CMedievalLogic*>(gEntList.FindEntityByClassname(NULL, "tf_logic_medieval"));
	if (pMedievalLogic)
	{
		m_bPlayingMedieval = true;
	}
	*/
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

// OFSTATUS: INCOMPLETE
void COFGameRules::CreateStandardEntities()
{
	//g_pPlayerResource

	g_pObjectiveResource = dynamic_cast<COFObjectiveResource*>(CBaseEntity::Create("tf_objective_resource", vec3_origin, vec3_angle));

	m_hOFGameRulesProxy = dynamic_cast< COFGameRulesProxy* >(CBaseEntity::Create("tf_gamerules", vec3_origin, vec3_angle));
}

// OFSTATUS: INCOMPLETE
void COFGameRules::RoundRespawn()
{
	//*(undefined *)&this->field_0xc8c = 0;
	//this->field_0xcb0 = 0;

	RemoveAllProjectilesAndBuildings(false);

	// not finished + gotta implement objects
	/*
	for (int i = 0; i < IBaseObjectAutoList::AutoList().count(); i++)
	{
	CBaseObject *pObj = static_cast<CBaseObject*>(IBaseObjectAutoList::AutoList()[i]);
	if (pObj && pObj->ObjectType() == 2 && ???? && pObj->GetTeamNumber() != m_iWinningTeam)
	{
	pObj->SetDisabled(false);
	}
	}
	*/

	if (!IsInTournamentMode())
	{
		//Arena_RunTeamLogic();
	}

	//field_0x698 = 0;

	int iTeams = OFTeamMgr()->GetTeamCount();
	for (int teamIndex = FIRST_GAME_TEAM; teamIndex < iTeams; teamIndex++)
	{
		COFTeam *pTeam = GetGlobalOFTeam(teamIndex);

		if (pTeam)
		{
			pTeam->SetFlagCaptures(0);
		}
	}

	IGameEvent *pEvent = gameeventmanager->CreateEvent("scorestats_accumulated_update");
	if (pEvent)
	{
		gameeventmanager->FireEvent(pEvent);
	}

	//CTFGameStats::ResetRoundStats((CTFGameStats *)PTR__CTF_GameStats_00e340b8);
	BaseClass::RoundRespawn();

	/*
	if (m_bForceMapReset || field_0x279)
	{
		for (int playerIndex = 1; playerIndex < gpGlobals->maxClients; playerIndex++)
		{
			COFPlayer *pPlayer = ToOFPlayer(UTIL_PlayerByIndex(playerIndex));
			if (pPlayer)
			{
				pPlayer->m_Shared.SetDefualtItemChargeMeters()
			}
		}
	}
	*/

	if (!IsInWaitingForPlayers())
	{
		//ShowRoundInfoPanel(false);
	}

	/*
	if (field_0x6d5)
	{
		g_voteController->CreateVote(99, "nextlevel", "");
		field_0x6d4 = 1;
	}
	*/
}

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
	if (m_nGameType == OF_GAMETYPE_CTF)
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

// OFSTATUS: INCOMPLETE
void COFGameRules::RestoreActiveTimer()
{
	BaseClass::RestoreActiveTimer();

	if (!IsInKothMode()) return;

	//if (m_hBlueKothTimer)

	//if (m_hRedKothTimer)
}

// OFSTATUS: COMPLETE
bool COFGameRules::RoundCleanupShouldIgnore(CBaseEntity *pEnt)
{
	if (FindInList(s_PreserveEnts, pEnt->GetClassname())) return true;

	if (Q_strstr(pEnt->GetClassname(), "tf_weapon_")) return true;

	return BaseClass::RoundCleanupShouldIgnore(pEnt);
}

// OFSTATUS: COMPLETE
bool COFGameRules::ShouldCreateEntity(char const *pszClassName)
{
	if (FindInList(s_PreserveEnts, pszClassName)) return false;

	return BaseClass::ShouldCreateEntity(pszClassName);
}

// OFSTATUS: INCOMPLETE
void COFGameRules::CleanUpMap()
{
	for (int playerIndex = 1; playerIndex < gpGlobals->maxClients; playerIndex++)
	{
		COFPlayer *pPlayer = ToOFPlayer(UTIL_PlayerByIndex(playerIndex));
		if (pPlayer)
		{
			//pPlayer->m_Shared.RemoveAllCond();
		}
	}

	BaseClass::CleanUpMap();

	if (HLTVDirector())
	{
		HLTVDirector()->BuildCameraList();
	}

	//*(undefined *)&this->field_0xc8c = 0;
	//this->field_0xcb0 = 0;
}


Vector DropToGround(CBaseEntity *pEntity, const Vector &vecPos, const Vector &vecMin, const Vector &vecMax)
{
	trace_t trace;
	UTIL_TraceHull(vecPos, vecPos + Vector(0.0, 0.0, -500.0), vecMin, vecMax, MASK_SOLID, pEntity, COLLISION_GROUP_NONE, &trace);
	return trace.endpos;
}

// OFSTATUS: COMPLETE
// removed training mode stuff
CBaseEntity *COFGameRules::GetPlayerSpawnSpot(CBasePlayer *pPlayer)
{
	CBaseEntity *pSpawnPoint = pPlayer->EntSelectSpawnPoint();

	Vector vecPoint = DropToGround(pPlayer, pSpawnPoint->GetAbsOrigin(), VEC_HULL_MIN_SCALED(pPlayer), VEC_HULL_MAX_SCALED(pPlayer));

	pPlayer->SetAbsOrigin(vecPoint + Vector(0.0, 0.0, 1.0));
	pPlayer->SetAbsVelocity(vec3_origin);
	pPlayer->SetLocalAngles(pSpawnPoint->GetAbsAngles());
	pPlayer->m_Local.m_vecPunchAngle = vec3_angle; // thank you gamerules.cpp
	pPlayer->m_Local.m_vecPunchAngleVel = vec3_angle;
	pPlayer->SnapEyeAngles(pSpawnPoint->GetAbsAngles());

	return pSpawnPoint;
}

// OFSTATUS: COMPLETE
// cut matchsummary stuff
// OFTODO: skipping the SpawnMode thing for now as i dont wanna go down another rabbit hole right now
// bDontIncludePlayers - when false it'll take in account if there is already a player in the area of the spawnpoint
bool COFGameRules::IsSpawnPointValid(CBaseEntity *pSpawnPoint,CBasePlayer *pPlayer, bool bDontIncludePlayers)
{
	if (pSpawnPoint->GetTeamNumber() != pPlayer->GetTeamNumber()) return false;

	if (!pSpawnPoint->IsTriggered(pPlayer)) return false;

	// spawnmode stuff happens around here
	COFTeamSpawn *pTeamSpawn = dynamic_cast<COFTeamSpawn*>(pSpawnPoint);
	if (pTeamSpawn && pTeamSpawn->IsDisabled()) return false;

	if (!bDontIncludePlayers)
	{
		return UTIL_IsSpaceEmpty(pPlayer, VEC_HULL_MIN_SCALED(pPlayer) + pSpawnPoint->GetAbsOrigin(), VEC_HULL_MAX_SCALED(pPlayer) + pSpawnPoint->GetAbsOrigin());
	}

	/*
	local_6c = *(float *)(param_1 + 0x2a4) + fVar13;
    local_68 = *(float *)(param_1 + 0x2a8) + fVar14;
    local_64 = *(float *)(param_1 + 0x2ac) + fVar11;
    local_4c = (uint)fVar13 ^ 0x80000000;
    local_48 = (uint)fVar14 ^ 0x80000000;
    local_44 = (uint)fVar11 ^ 0x80000000;
	*/

	// 0x1 + 0x4000 + 0x10000 + 0x2 + 0x2000000 + 0x8 = 0x201400B = MASK_PLAYERSOLID

	trace_t trace;
	UTIL_TraceHull(pSpawnPoint->GetAbsOrigin(), pSpawnPoint->GetAbsOrigin(), VEC_HULL_MIN_SCALED(pPlayer), VEC_HULL_MAX_SCALED(pPlayer), MASK_PLAYERSOLID, pPlayer, COLLISION_GROUP_NONE, &trace);
	return (!trace.startsolid && !trace.allsolid && trace.fraction);
}

// OFSTATUS: COMPLETE
// this is hurting my head so much - cherry
void COFGameRules::RecalculateControlPointState()
{
	if (g_hControlPointMasters.Count() && (!g_pObjectiveResource || !g_pObjectiveResource->PlayingMiniRounds()))
	{
		for (int iTeam = FIRST_GAME_TEAM; iTeam < GetNumberOfTeams(); iTeam++)
		{
			int iFarthestCP = GetFarthestOwnedControlPoint(iTeam, true);

			for (int iControlPoint = 0; iControlPoint < IOFTeamSpawnAutoList::AutoList().Count(); iControlPoint++)
			{
				COFTeamSpawn *pOFSpawn = dynamic_cast<COFTeamSpawn*>(IOFTeamSpawnAutoList::AutoList()[iControlPoint]);
				if (pOFSpawn->GetControlPoint())
				{
					if (pOFSpawn->GetTeamNumber() == iTeam)
					{
						if (pOFSpawn->GetControlPoint()->GetPointIndex() == iFarthestCP)
						{
							pOFSpawn->SetDisable(false);
						}
						else
						{
							pOFSpawn->SetDisable(true);
						}
					}
				}
			}
		}
	}
}

// OFSTATUS: INCOMPLETE
void COFGameRules::SetupOnRoundStart()
{
	for (int i = 0; i < MAX_TEAMS; i++)
	{
		g_pObjectiveResource->SetBaseCP(-1, i);
	}

	for (int i = 0; i < OF_TEAM_COUNT; i++)
	{
		//field_0x654 .. field_0x650 .. field_0x65c .. field_0x658[i] = 0;
	}

	SetOvertime(false);

	m_hRedKothTimer = -1;
	m_hBlueKothTimer = -1;

	CBaseEntity *pEntity = gEntList.FirstEnt();
	while (pEntity)
	{
		variant_t emptyVariant;
		pEntity->AcceptInput("RoundSpawn", NULL, NULL, emptyVariant, 0);

		pEntity = gEntList.NextEnt(pEntity);
	}

	//*(undefined *)&this->field_0x50c = 0;
	//this->field_0xc58 = 0;
	//this->field_0xc70 = 0;

	pEntity = gEntList.FirstEnt();
	while (pEntity)
	{
		variant_t emptyVariant;
		pEntity->AcceptInput("RoundActivate", NULL, NULL, emptyVariant, 0);

		pEntity = gEntList.NextEnt(pEntity);
	}

	if (g_pObjectiveResource && !g_pObjectiveResource->PlayingMiniRounds())
	{
		memset(field_0xb3c, 0, MAX_TEAMS * MAX_CONTROL_POINTS);
		for (int i = 0; i < IOFTeamSpawnAutoList::AutoList().Count(); i++)
		{
			COFTeamSpawn *pOFSpawn = dynamic_cast<COFTeamSpawn*>(IOFTeamSpawnAutoList::AutoList()[i]);
			if (pOFSpawn->GetControlPoint())
			{
				int iTeam = pOFSpawn->GetTeamNumber();
				int iPointIndex = pOFSpawn->GetControlPoint()->GetPointIndex();
				field_0xb3c[iTeam][iPointIndex] = true;
				pOFSpawn->SetDisable(true);
			}
		}
		
		RecalculateControlPointState();
		SetRoundOverlayDetails();
	}

	// training mode stuff
	//if (IsInTraining() && field_0x6a4) { field_0x6a4->0x524 = 0 }

	field_0x62c[0] = '\0';

	// halloween, mvm, stuff
	//field_0xc98 = 0xbf800000;
	//field_0xce8 = 0;
	//field_0xcc8 = 0xbf800000;
	//field_0xccc = 0;
	//SetIT()
	// ...
}

// OFSTATUS: COMPLETE
void COFGameRules::SetupOnRoundRunning()
{
	for (int i = 0; i < g_hControlPointMasters.Count(); i++)
	{
		CTeamControlPointMaster *pControlPoint = g_hControlPointMasters[i];
		if (pControlPoint)
		{
			variant_t emptyVariant;
			pControlPoint->AcceptInput("RoundStart", NULL, NULL, emptyVariant, 0);
		}
	}

	for (int i = 1; i < gpGlobals->maxClients; i++)
	{
		COFPlayer *pPlayer = ToOFPlayer(UTIL_PlayerByIndex(i));
		if (pPlayer)
		{
			pPlayer->SetSpeedOF(); // TeamFortress_SetSpeed();
			// there was a bunch of weird checks going on here on hallowen and steam id stuff, so ignoring all that its just this:
			pPlayer->SpeakConceptIfAllowed(MP_CONCEPT_ROUND_START);
		}
	}

	if (IsPlayingSpecialDeliveryMode() && !IsInWaitingForPlayers())
	{
		BroadcastSound(255, "Announcer.SD_RoundStart");
	}
}

// OFSTATUS: COMPLETE
void COFGameRules::PreviousRoundEnd()
{
	if (g_hControlPointMasters.Count() && g_hControlPointMasters[0])
	{
		g_hControlPointMasters[0]->FireRoundEndOutput();
	}

	m_iPreviousRoundWinnerTeam = GetWinningTeam();
}

// OFSTATUS: COMPLETE
void COFGameRules::SetupSpawnPointsForRound()
{
	// 0x398 = m_ControlPointRounds.Count() ?
	// 0x3a0 = GetCurrentRound() ?
	CTeamControlPointRound *pControlPointRound = g_hControlPointMasters[0]->GetCurrentRound();
	if (g_hControlPointMasters.Count() && g_hControlPointMasters[0] && (g_hControlPointMasters[0]->PlayingMiniRounds()) && pControlPointRound)
	{
		for (int i = 0; i < IOFTeamSpawnAutoList::AutoList().Count(); i++)
		{
			COFTeamSpawn *pOFSpawn = dynamic_cast<COFTeamSpawn*>(IOFTeamSpawnAutoList::AutoList()[i]);

			if (pOFSpawn)
			{
				CHandle<CTeamControlPoint> pControlPoint = pOFSpawn->GetControlPoint();
				CHandle<CTeamControlPointRound> pBlueControlPoint = pOFSpawn->GetBlueControlPoint();
				CHandle<CTeamControlPointRound> pRedControlPoint = pOFSpawn->GetRedControlPoint();

				if (pControlPoint && pControlPointRound->IsControlPointInRound(pControlPoint))
				{
					pOFSpawn->SetDisable(false);
					pOFSpawn->ChangeTeam(pControlPoint->GetOwner());
				}
				else if (pBlueControlPoint && pBlueControlPoint == pControlPointRound)
				{
					pOFSpawn->SetDisable(false);
					pOFSpawn->ChangeTeam(OF_TEAM_BLUE);
				}
				else if (pRedControlPoint && pRedControlPoint == pControlPointRound)
				{
					pOFSpawn->SetDisable(false);
					pOFSpawn->ChangeTeam(OF_TEAM_RED);
				}
				else
				{
					pOFSpawn->SetDisable(true);
				}
			}
		}
	}
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

// OFSTATUS: COMPLETE
const char *COFGameRules::GetStalemateSong(int nTeam)
{
	if (m_bPlayingSpecialDeliveryMode) return "Announcer.SD_Event_MurderedToStalemate";

	return BaseClass::GetStalemateSong(NULL);
}

// OFSTATUS: COMPLETE
void COFGameRules::SetHudType(int iHudType)
{
	// OFTODO: figure out these hud types
	if (iHudType != OF_HUDTYPE_UNDEFINDED)
	{
		if (iHudType != OF_HUDTYPE_UNKNOWN4 && iHudType < OF_HUDTYPE_UNKNOWN6)
		{
			m_nHudType = iHudType;
		}
	}
}

#endif // GAME_DLL

// OFSTATUS: COMPLETE
// cheese and crackers - cherry
int COFGameRules::GetFarthestOwnedControlPoint(int iTeam, bool param_2)
{
	int iBaseTeamCP = g_pObjectiveResource->GetBaseControlPointForTeam(iTeam);

	if (iBaseTeamCP == -1) return -1;

	int iAddition = -1;
	int iCPAmount = 0;
	if (iBaseTeamCP == 0)
	{
		iAddition = 1;
		iCPAmount = g_pObjectiveResource->GetNumControlPoints() - 1;
	}

	int iFarthestCP = iBaseTeamCP;
	for (int i = iBaseTeamCP; i != iCPAmount; i += iAddition)
	{
		if (g_pObjectiveResource->GetOwningTeam(i) != iTeam) break;

		if (param_2 && !field_0xb3c[iTeam][i]) continue;

		iFarthestCP = i;
	}

	return iFarthestCP;
}

// OFSTATUS: COMPLETE
bool COFGameRules::TeamMayCapturePoint(int iTeam, int iPointIndex)
{
	if (!tf_caplinear.GetBool()) return true;

	int iPreviousPoint = g_pObjectiveResource->GetPreviousPointForPoint(iPointIndex, iTeam, 0);
	if (iPreviousPoint == iPointIndex) return true;

	if (IsInKothMode() && IsInWaitingForPlayers()) return false;

	if (g_pObjectiveResource->GetCPLocked(iPointIndex)) return false;

	if (iPreviousPoint == -1)
	{
		if (IsInArenaMode())
		{
			// field_0x93c = m_flCapturePointEnableTime
			if (gpGlobals->curtime >= m_flCapturePointEnableTime && State_Get() == GR_STATE_STALEMATE) return true;

			return false;
		}
		else
		{
			if (!g_pObjectiveResource->PlayingMiniRounds())
			{
				int iFarthestCP = GetFarthestOwnedControlPoint(iTeam, false);
				return (abs(iFarthestCP - iPointIndex) <= 1);
			}

			return true;
		}
	}

	for (int i = 0; i < MAX_PREVIOUS_POINTS; i++)
	{
		int iPreviousPoint = g_pObjectiveResource->GetPreviousPointForPoint(iPointIndex, iTeam, i);
		if (iPreviousPoint != -1 && g_pObjectiveResource->GetOwningTeam(iPreviousPoint) != iTeam)
		{
			return false;
		}
	}

	return true;
}

// OFSTATUS: COMPLETE
bool COFGameRules::PlayerMayCapturePoint(CBasePlayer *pPlayer, int iPointIndex, char *pszReason, int iMaxReasonLength)
{
	// OFTODO: gotta implement player conditions
	/*
	COFPlayer *pOFPlayer = ToOFPlayer(pPlayer);

	// cant cap when invisible (spy)
	if (pOFPlayer->m_Shared.IsStealthed())
	{
		if (pszReason)
		{
			V_snprintf(pszReason, iMaxReasonLength, "#Cant_cap_stealthed");
		}

		return false;
	}

	// cant cap when ubered
	if (pOFPlayer->m_Shared.IsInvulnerable())
	{
		if (pszReason)
		{
			V_snprintf(pszReason, iMaxReasonLength, "#Cant_cap_invuln");
		}

		return false;
	}

	// cant cap when disguised
	if (pOFPlayer->m_Shared.InCond(TF_COND_DISGUISED) && pOFPlayer->m_Shared.GetDisguiseTeam() != pOFPlayer->GetTeamNumber())
	{
		if (pszReason)
		{
			V_snprintf(pszReason, iMaxReasonLength, "#Cant_cap_disguised");
		}

		return false;
	}
	*/

	// training mode stuff, line 54 - 83
	//if (IsInTraining())
	//{}

	return true;
}

// OFSTATUS: COMPLETE
bool COFGameRules::PlayerMayBlockPoint(CBasePlayer *pPlayer, int iPointIndex, char *pszReason, int iMaxReasonLength)
{
	// OFTODO: gotta implement player conditions
	/*
	COFPlayer *pOFPlayer = ToOFPlayer(pPlayer);

	if (pOFPlayer->m_Shared.IsInvulnerable())
	{
		if (pszReason)
		{
			V_snprintf(pszReason, iMaxReasonLength, "#Cant_cap_invuln");
		}

		return true;
	}
	*/

	return false;
}

// OFSTATUS: COMPLETE
bool COFGameRules::IsPlayingSpecialDeliveryMode()
{
	return m_bPlayingSpecialDeliveryMode;
}

// OFSTATUS: COMPLETE
// taken from CTeamplayRoundBasedRules, it was a server only function though
int COFGameRules::GetTimeLeft()
{
	float flTimeLimit = mp_timelimit.GetInt() * 60;
	float flMapChangeTime = m_flMapResetTime + flTimeLimit;

	int iTime = (int)(flMapChangeTime - gpGlobals->curtime);
	if (iTime < 0)
	{
		iTime = 0;
	}

	return (iTime);
}

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

ConVar ammo_max( "ammo_max", "5000", FCVAR_GAMEDLL | FCVAR_REPLICATED );

#ifdef GAME_DLL
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
	
		int iAmmoIndex = OF_AMMO_PRIMARY;
		do
		{
			def.AddAmmoType( g_aAmmoNames[iAmmoIndex], DMG_BULLET | DMG_NOCLOSEDISTANCEMOD | DMG_USEDISTANCEMOD, 1,	0, 0, ammo_max.GetInt(), 2400.0, 10, 14, 8 );
			iAmmoIndex++;
		}
		while (iAmmoIndex != OF_AMMO_COUNT);
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
	int iTeam = OF_TEAM_RED; // TEAM_COMBINE;

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