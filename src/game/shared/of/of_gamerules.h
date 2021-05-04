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

class COFGameRulesProxy : public CTeamplayRoundBasedRulesProxy
{
public:
	DECLARE_CLASS( COFGameRulesProxy, CTeamplayRoundBasedRulesProxy );
	DECLARE_NETWORKCLASS();
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
	virtual void CreateStandardEntities(void);
	virtual void Think(void);
#endif
	virtual void LevelInitPostEntity();
	virtual float GetRespawnTimeScalar(int iTeam);
	virtual float GetRespawnWaveMaxLength(int iTeam, bool bScaleWithNumPlayers);
	virtual bool FlagsMayBeCapped();
#ifdef GAME_DLL
	virtual void RemoveAllProjectiles();
	virtual void RemoveAllBuildings(bool param_1);
	virtual void RemoveAllSentriesAmmo();
	virtual void RemoveAllProjectilesAndBuildings(bool param_1);
	virtual bool CanChangelevelBecauseOfTimeLimit();
	virtual bool CanGoToStalemate();
	virtual bool CheckCapsPerRound();
	virtual bool SetCtfWinningTeam();
	virtual bool CanFlagBeCaptured(COFPlayer *pPlayer);
#endif
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
	void TeamPlayerCountChanged(COFTeam *);
#endif

	virtual bool IsPlayingSpecialDeliveryMode( void );

private:

	CHandle<COFGameRulesProxy>	m_hOFGameRulesProxy;
	CNetworkVar(int, m_nGameType);

};

inline COFGameRules* OFGameRules()
{
	return static_cast< COFGameRules* >( g_pGameRules );
}
