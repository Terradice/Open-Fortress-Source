// ========= Copyright Open Fortress Developers, CC-BY-NC-SA ============
// Purpose: Game logic manager
// Author(s): ficool2, Fenteale, Nopey
//
#pragma once

//#include "gamerules.h"
#include "teamplayroundbased_gamerules.h"
//#include "GameEventListener.h"
//#include "teamplay_gamerules.h"
#include "gamevars_shared.h"

#ifdef CLIENT_DLL
#include "c_of_player.h"
#else
#include "of_player.h"
#endif

#ifdef CLIENT_DLL
	#define COFGameRules C_OFGameRules
	#define COFGameRulesProxy C_OFGameRulesProxy
#endif

#ifdef GAME_DLL
class COFTeam;
#endif

#define HUD_ALERT_SCRAMBLE_TEAMS	0

class COFGameRulesProxy : public CTeamplayRoundBasedRulesProxy , public CGameEventListener
{
public:
	DECLARE_CLASS( COFGameRulesProxy, CTeamplayRoundBasedRulesProxy );
	DECLARE_NETWORKCLASS();

#ifdef GAME_DLL
	DECLARE_DATADESC();

	COFGameRulesProxy();
	virtual void Activate();

	virtual void SetRedTeamRespawnWaveTime(inputdata_t &inputdata);

private:

	int m_nHudType;
	bool m_bOvertimeAllowedForCTF;

	COutputEvent m_OnWonByTeam1;
	COutputEvent m_OnWonByTeam2;

#endif

public:
	virtual void FireGameEvent(IGameEvent * event);
};

class COFGameRules : public CTeamplayRoundBasedRules
{
public:
	DECLARE_CLASS( COFGameRules, CTeamplayRoundBasedRules );

	 // This makes datatables able to access our private vars
	DECLARE_NETWORKCLASS_NOBASE();
	
	COFGameRules();
	virtual ~COFGameRules();

#ifdef GAME_DLL
	virtual void Precache( void );
	virtual void Think(void);
	virtual void Activate();
#endif
	virtual void LevelInitPostEntity();
	virtual float GetRespawnTimeScalar(int iTeam);
	virtual float GetRespawnWaveMaxLength(int iTeam, bool bScaleWithNumPlayers);
	virtual bool FlagsMayBeCapped();
#ifdef GAME_DLL
	virtual void CreateStandardEntities();
	virtual void RoundRespawn();
	virtual void RemoveAllProjectiles();
	virtual void RemoveAllBuildings(bool param_1);
	virtual void RemoveAllSentriesAmmo();
	virtual void RemoveAllProjectilesAndBuildings(bool param_1);
	virtual bool CanChangelevelBecauseOfTimeLimit();
	virtual bool CanGoToStalemate();
	virtual bool CheckCapsPerRound();
	virtual void RestoreActiveTimer();
	virtual bool RoundCleanupShouldIgnore(CBaseEntity *pEnt);
	virtual bool ShouldCreateEntity(const char *pszClassName);
	virtual void CleanUpMap();
	virtual CBaseEntity *GetPlayerSpawnSpot(CBasePlayer *pPlayer);
	virtual bool IsSpawnPointValid(CBaseEntity *pSpawnPoint, CBasePlayer *pPlayer, bool bDontIncludePlayers);
	virtual void RecalculateControlPointState();
	virtual void SetupOnRoundStart();
	virtual void SetupOnRoundRunning();
	virtual void PreviousRoundEnd();
	virtual void SetupSpawnPointsForRound();
	virtual bool SetCtfWinningTeam();
	virtual bool CanFlagBeCaptured(COFPlayer *pPlayer);
	virtual const char *GetStalemateSong(int nTeam);
	virtual void SetHudType(int iHudType);
	virtual void SetOvertimeAllowedForCTF(bool bAllow) { m_bOvertimeAllowedForCTF = bAllow; }
	virtual float FlItemRespawnTime() { return 10.0; } // param_1 goes unused
#endif
	virtual int GetFarthestOwnedControlPoint(int iTeam, bool param_2);
	virtual bool TeamMayCapturePoint(int iTeam, int iPointIndex);
	virtual bool PlayerMayCapturePoint(CBasePlayer *pPlayer, int iPointIndex, char *pszReason = NULL, int iMaxReasonLength = 0);
	virtual bool PlayerMayBlockPoint(CBasePlayer *pPlayer, int iPointIndex, char *pszReason = NULL, int iMaxReasonLength = 0);
	virtual bool ShouldCollide( int collisionGroup0, int collisionGroup1 );
	virtual bool ClientCommand( CBaseEntity *pEdict, const CCommand &args );
	virtual int PlayerRelationship( CBaseEntity *pPlayer, CBaseEntity *pTarget );
	virtual void GoToIntermission( void );
	virtual void DeathNotice( CBasePlayer *pVictim, const CTakeDamageInfo &info );
	virtual const char *GetGameDescription( void );
	virtual const unsigned char *GetEncryptionKey( void ) { return (unsigned char *)"E2NcUkG2"; }
	virtual void ClientDisconnected( edict_t *pClient );
	virtual bool IsConnectedUserInfoChangeAllowed( CBasePlayer *pPlayer );
#ifdef GAME_DLL
	virtual void TeamPlayerCountChanged(COFTeam *pTeam);
#endif

	virtual bool IsPlayingSpecialDeliveryMode( void );
	virtual int GetTimeLeft();

private:

	CHandle<COFGameRulesProxy>	m_hOFGameRulesProxy;
	CNetworkVar(int, m_nHudType);
	CNetworkVar(int, m_nGameType);
	CNetworkVar(bool, m_bPlayingKoth);
	CNetworkVar(bool, m_bPlayingMedieval);
	CNetworkVar(bool, m_bPlayingSpecialDeliveryMode);

	CNetworkVar(float, m_hRedKothTimer);
	CNetworkVar(float, m_hBlueKothTimer);

	CNetworkVar(float, m_flCapturePointEnableTime);

	bool field_0xb3c[MAX_TEAMS][MAX_CONTROL_POINTS];

	#ifdef GAME_DLL

	bool m_bOvertimeAllowedForCTF;
	char field_0x62c[256]; // figure out the proper size later
	int m_iPreviousRoundWinnerTeam;

	#endif

};

inline COFGameRules* OFGameRules()
{
	return static_cast< COFGameRules* >( g_pGameRules );
}
