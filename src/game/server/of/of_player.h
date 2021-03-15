// ========= Copyright Open Fortress Developers, CC-BY-NC-SA ============
// Purpose: Implementation of COFPlayer
// Author(s): Fenteale, Nopey
//

#pragma once

#include "basemultiplayerplayer.h"
#include "dbg.h"
#include "of_playeranimstate.h"
#pragma once

#include "basemultiplayerplayer.h"
#include "dbg.h"
#include "of_playeranimstate.h"
#include "of_player_shared.h"

class COFWeaponBase;

enum OFPlayerState
{
	TF_STATE_ACTIVE,
	TF_STATE_WELCOME,
	TF_STATE_OBSERVER,
	TF_STATE_DYING,
	TF_STATE_LAST
};

extern const char* sz_OFPlayerState[TF_STATE_LAST];

class COFPlayer : public CBaseMultiplayerPlayer {
public:
	DECLARE_CLASS( COFPlayer, CBaseMultiplayerPlayer );
	DECLARE_DATADESC();
	DECLARE_SERVERCLASS();

	// This passes the event to the client's and server's CPlayerAnimState.
	void DoAnimationEvent( PlayerAnimEvent_t event, int nData = 0 );

	COFPlayer();
	static COFPlayer* CreatePlayer( const char * name, edict_t* pEdict);

	virtual void InitialSpawn() override;

	void StateEnter( OFPlayerState state );

	virtual void StateEnterWELCOME();

	virtual void Spawn() override;
	virtual void ForceRespawn() override;

	virtual bool IsValidObserverTarget( CBaseEntity* target ) override { return true; }

	// Called from of_gamerules
	void PreCacheKart();
	static void PreCacheMvM();


	void Precache();
	void PrecacheOFPlayer();
	void PrecachePlayerModels();
	bool ClientCommand( const CCommand& args );
	void HandleCommand_JoinTeam(const char* arg);
	void ChangeTeam(int iTeam);
	void UpdateModel();
	CBaseEntity *EntSelectSpawnPoint();
	CBaseEntity* SelectSpawnSpotByType(char * type, CBaseEntity **param_2);
	
	COFWeaponBase 	*GetActiveOFWeapon( void ) const;
	bool			ShouldAutoReload(){ return false; };
	
	virtual void FireBullet( 
						   Vector vecSrc,	// shooting postion
						   const QAngle &shootAngles,  //shooting angle
						   float vecSpread, // spread vector
						   int iDamage, // base damage
						   int iBulletType, // ammo type
						   CBaseEntity *pevAttacker, // shooter
						   bool bDoEffects,	// create impact effect ?
						   float x,	// spread x factor
						   float y	// spread y factor
						   );


	// set to true from of_gamerules
	//OFTODO: I'm assuming the init value of m_bOFPlayerNeedsPrecache is false.
	static bool m_bOFPlayerNeedsPrecache;

	// Tracks our ragdoll entity.
	CNetworkHandle( CBaseEntity, m_hRagdoll );	// networked entity handle 	
	
	CNetworkVarEmbedded(COFPlayerShared, m_Shared);
	friend class COFPlayerShared;

	virtual float GetCritMult() { return m_Shared.GetCritMult(); };

private:
	COFPlayerAnimState *m_PlayerAnimState;
	OFPlayerState	m_iPlayerState;
};

inline COFPlayer *ToOFPlayer( CBaseEntity *pEntity )
{
	if ( !pEntity )
		return nullptr;
	
	if( !pEntity->IsPlayer() )
		return nullptr;

	return static_cast< COFPlayer* >( pEntity );
}