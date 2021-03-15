// ========= Copyright Open Fortress Developers, CC-BY-NC-SA ============
// Purpose: Implementation of CTFPlayer
// Author(s): Fenteale
//

// TODO: Check against tf2 server.dylib! Much lacking functionality. -@Nopey
// (Many functions are copied from sdk_player.cpp. Please mark any that match tf2 server.dylib)

#include "cbase.h"
#include "of_player.h"
#include "of_player_shared.h"
#include "of_playeranimstate.h"
#include "of_shareddefs.h"
#include "tier0/vprof.h"
#include "entity_ofstart.h"
#include "viewport_panel_names.h"
#include "effect_dispatch_data.h"
#include "te_effect_dispatch.h"
#include "engine/ivdebugoverlay.h"
#include "of_weapon_base.h"


ConVar sv_motd_unload_on_dismissal( "sv_motd_unload_on_dismissal", "0", 0, "If enabled, the MOTD contents will be unloaded when the player closes the MOTD." );
ConVar tf_playerstatetransitions( "tf_playerstatetransitions", "-2", FCVAR_DEVELOPMENTONLY, "tf_playerstatetransitions <ent index or -1 for all>. Show player state transitions." );

const char* sz_OFPlayerState[TF_STATE_LAST] =
{
    "TF_STATE_ACTIVE",
	"TF_STATE_WELCOME",
	"TF_STATE_OBSERVER",
    "TF_STATE_DYING"
};

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

LINK_ENTITY_TO_CLASS( player, COFPlayer );

IMPLEMENT_SERVERCLASS_ST(COFPlayer, DT_OF_Player)
END_SEND_TABLE()

BEGIN_DATADESC( COFPlayer )
END_DATADESC()

COFPlayer::COFPlayer() : BaseClass()
{
	m_PlayerAnimState = CreatePlayerAnimState(this);
}

bool COFPlayer::m_bOFPlayerNeedsPrecache { false };

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
void COFPlayer::StateEnter( OFPlayerState state )
{
	if ( m_iPlayerState != state )
		m_iPlayerState = state;

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
    RemoveSolidFlags( FSOLID_NOT_SOLID );
    m_hRagdoll = NULL;

    if ( m_iPlayerState == TF_STATE_WELCOME )
    {
        StateEnterWELCOME();
    }

}

//OFSTATUS: Incomplete, placeholder
void COFPlayer::ForceRespawn()
{
    StateEnter( TF_STATE_ACTIVE );
    BaseClass::ForceRespawn();
}

void COFPlayer::DoAnimationEvent( PlayerAnimEvent_t event, int nData )
{
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
    PrecacheModel("models/player/scout.mdl"); //TEMPORARY FOR TESTING ONLY
}


//OFSTATUS: Incomplete, only handles jointeam and in jointeam it only handles actual numbers.
bool COFPlayer::ClientCommand( const CCommand& args )
{
	if ( FStrEq( args[0], "jointeam" ) && args.ArgC() >= 2 )
	{
        HandleCommand_JoinTeam(args[1]);
		return true;
	}

	return BaseClass::ClientCommand( args );
}

//OFSTATUS: Incomplete, all placeholder
void COFPlayer::HandleCommand_JoinTeam(const char* arg)
{
    int iTeam = atoi(arg);

    if (iTeam != GetTeamNumber())
    {
        ChangeTeam(iTeam);
        ForceRespawn();
    }
}

//OFSTATUS: Incomplete, all placeholder
void COFPlayer::ChangeTeam(int iTeam)
{
    switch (iTeam)
    {
        case OF_TEAM_RED: m_nSkin = 0; break;
        case OF_TEAM_BLUE: m_nSkin = 1; break;
        default: m_nSkin = 0;
    }
    UpdateModel();
    BaseClass::ChangeTeam(iTeam);
}

//OFSTATUS: Incomplete, all placeholder
CBaseEntity *COFPlayer::EntSelectSpawnPoint()
{
    return SelectSpawnSpotByType("info_player_teamspawn", nullptr);

    //return BaseClass::EntSelectSpawnPoint();
}

//OFSTATUS: Incomplete, all placeholder
CBaseEntity* COFPlayer::SelectSpawnSpotByType(char* type, CBaseEntity** param_2)
{
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
}

//OFSTATUS: Incomplete, needs CTFPlayerClassShared to get the model names
void COFPlayer::UpdateModel()
{
	//BaseClass::SetModel((COFPlayerClassShared*)(this[1].data + 0x688)->GetModelName());
	BaseClass::SetModel("models/player/scout.mdl"); //TEMPORARY FOR TESTING ONLY
	SetCollisionBounds(BaseClass::GetPlayerMins(), BaseClass::GetPlayerMaxs());
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