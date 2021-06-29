// ========= Copyright Open Fortress Developers, CC-BY-NC-SA ============
// Purpose: Implementation of CCaptureFlag
// Author(s): Cherry!
//

#include "cbase.h"
#include "of_flag.h"
#include "filesystem.h"
#include "datacache/imdlcache.h"
#include "of_gamerules.h"
//#include "of_shareddefs.h"

#ifdef CLIENT_DLL
#include "c_of_player.h"
#include "c_of_team.h"
#else
#include "of_player.h"
#include "SpriteTrail.h"
#include "of_team.h"
#endif

#define OF_FLAG_HUDICON "../hud/objectives_flagpanel_carried"
#define OF_FLAG_MODEL "models/flag/briefcase.mdl"
#define OF_FLAG_PAPEREFFECT "player_intel_papertrail"

ConVar tf_flag_caps_per_round("tf_flag_caps_per_round", "3", FCVAR_REPLICATED, "Number of captures per round on CTF maps. Set to 0 to disable.");
ConVar tf_flag_return_on_touch("tf_flag_return_on_touch", "0", FCVAR_REPLICATED, "If this is set, your flag must be at base in order to capture the enemy flag. Remote friendly flags return to your base instantly when you touch them.");
ConVar cl_flag_return_height("cl_flag_return_height", "82", FCVAR_CHEAT);

IMPLEMENT_AUTO_LIST(ICaptureFlagAutoList);

IMPLEMENT_NETWORKCLASS_ALIASED(CaptureFlag, DT_CaptureFlag)

#ifdef CLIENT_DLL

static void RecvProxy_IsDisabled(const CRecvProxyData *pData, void *pStruct, void *pOut)
{
	CCaptureFlag *pFlag = (CCaptureFlag *)pStruct;

	if (pFlag)
	{
		pFlag->SetDisabled(0 < (pData->m_Value.m_Int));
	}
}

static void RecvProxy_IsVisibleWhenDisabled(const CRecvProxyData *pData, void *pStruct, void *pOut)
{
	CCaptureFlag *pFlag = (CCaptureFlag *)pStruct;

	if (pFlag)
	{
		pFlag->SetVisibleWhenDisabled(pData->m_Value.m_Int);
	}
}

static void RecvProxy_FlagStatus(const CRecvProxyData *pData, void *pStruct, void *pOut)
{
	CCaptureFlag *pFlag = (CCaptureFlag *)pStruct;

	if (pFlag)
	{
		pFlag->SetFlagStatus(pData->m_Value.m_Int);

		// C_CaptureFlag::UpdateGlowEffect();

		IGameEvent *pEvent = gameeventmanager->CreateEvent("flagstatus_update");
		if (pEvent)
		{
			pEvent->SetInt("entindex", pFlag->entindex());
			gameeventmanager->FireEventClientSide(pEvent);
		}
	}
}
#endif

BEGIN_NETWORK_TABLE(CCaptureFlag, DT_CaptureFlag)
#ifdef CLIENT_DLL
	RecvPropInt(RECVINFO(m_bDisabled),0,RecvProxy_IsDisabled),
	RecvPropInt(RECVINFO(m_bVisibleWhenDisabled),0,RecvProxy_IsVisibleWhenDisabled),
	RecvPropInt(RECVINFO(m_nType)),
	RecvPropInt(RECVINFO(m_nFlagStatus),0,RecvProxy_FlagStatus),
	RecvPropTime(RECVINFO(m_flResetTime)),
	RecvPropTime(RECVINFO(m_flNeutralTime)),
	RecvPropTime(RECVINFO(m_flMaxResetTime)),
	RecvPropEHandle(RECVINFO(m_hPrevOwner)),
	RecvPropString(RECVINFO(m_szModel)),
	RecvPropString(RECVINFO(m_szHudIcon)),
	RecvPropString(RECVINFO(m_szPaperEffect)),
	RecvPropString(RECVINFO(m_szTrailEffect)),
	RecvPropInt(RECVINFO(m_nUseTrailEffect)),
#else
	SendPropInt(SENDINFO(m_bDisabled)),
	SendPropInt(SENDINFO(m_bVisibleWhenDisabled)),
	SendPropInt(SENDINFO(m_nType),5,SPROP_UNSIGNED),
	SendPropInt(SENDINFO(m_nFlagStatus),3,SPROP_UNSIGNED),
	SendPropTime(SENDINFO(m_flResetTime)),
	SendPropTime(SENDINFO(m_flNeutralTime)),
	SendPropTime(SENDINFO(m_flMaxResetTime)),
	SendPropEHandle(SENDINFO(m_hPrevOwner)),
	SendPropString(SENDINFO(m_szModel)),
	SendPropString(SENDINFO(m_szHudIcon)),
	SendPropString(SENDINFO(m_szPaperEffect)),
	SendPropString(SENDINFO(m_szTrailEffect)),
	SendPropInt(SENDINFO(m_nUseTrailEffect),3,SPROP_UNSIGNED),
#endif
END_NETWORK_TABLE()

BEGIN_DATADESC(CCaptureFlag)
	DEFINE_KEYFIELD(m_nType, FIELD_INTEGER, "GameType"),
	DEFINE_KEYFIELD(m_nReturnTime, FIELD_INTEGER, "ReturnTime"),
	DEFINE_KEYFIELD(m_nNeutralType, FIELD_INTEGER, "NeutralType"),
	DEFINE_KEYFIELD(m_nScoringType, FIELD_INTEGER, "ScoringType"),
	DEFINE_KEYFIELD(m_nUseTrailEffect, FIELD_INTEGER, "trail_effect"),
	DEFINE_KEYFIELD(m_bVisibleWhenDisabled, FIELD_BOOLEAN, "VisibleWhenDisabled"),
	//DEFINE_KEYFIELD(?????, FIELD_BOOLEAN, "ShotClockMode"),
	
	// MvM
	//DEFINE_KEYFIELD(?????, FIELD_INTEGER, "PointValue"),
	//DEFINE_KEYFIELD(?????, FIELD_BOOLEAN, "ReturnBetweenWaves"),
	//DEFINE_KEYFIELD(?????, FIELD_STRING, "tags"),

#ifdef GAME_DLL
	DEFINE_KEYFIELD(m_iszModel, FIELD_STRING, "flag_model"),
	DEFINE_KEYFIELD(m_iszHudIcon, FIELD_STRING, "flag_icon"),
	DEFINE_KEYFIELD(m_iszPaperEffect, FIELD_STRING, "flag_paper"),
	DEFINE_KEYFIELD(m_iszTrailEffect, FIELD_STRING, "flag_trail"),

	// Input
	DEFINE_INPUTFUNC(FIELD_VOID, "Enable", InputEnable),
	DEFINE_INPUTFUNC(FIELD_VOID, "Disable", InputDisable),
	/*
	DEFINE_INPUT(FIELD_VOID, "ForceDrop", InputForceDrop),
	DEFINE_INPUT(FIELD_VOID, "ForceReset", InputForceReset),
	DEFINE_INPUT(FIELD_VOID, "ForceResetSilent", InputForceResetSilent),
	DEFINE_INPUT(FIELD_VOID, "ForceResetAndDisableSilent", InputForceResetAndDisableSilent),
	*/
	DEFINE_INPUTFUNC(FIELD_INTEGER, "SetReturnTime", InputSetReturnTime),
	/*
	DEFINE_INPUT(FIELD_INTEGER, "ShowTimer", InputShowTimer),
	DEFINE_INPUT(FIELD_INTEGER, "ForceGlowDisabled", InputForceGlowDisabled),
	*/
	// Outputs
	DEFINE_OUTPUT(m_outputOnReturn, "OnReturn"),
	DEFINE_OUTPUT(m_outputOnPickUp, "OnPickup"),
	DEFINE_OUTPUT(m_outputOnPickUpTeam1, "OnPickupTeam1"),
	DEFINE_OUTPUT(m_outputOnPickUpTeam2, "OnPickupTeam2"),
	DEFINE_OUTPUT(m_outputOnDrop, "OnDrop"),
	DEFINE_OUTPUT(m_outputOnCapture, "OnCapture"),
	DEFINE_OUTPUT(m_OnCapTeam1, "OnCapTeam1"),
	DEFINE_OUTPUT(m_OnCapTeam2, "OnCapTeam2"),
	DEFINE_OUTPUT(m_OnTouchSameTeam, "OnTouchSameTeam"),
#endif
END_DATADESC();

LINK_ENTITY_TO_CLASS(item_teamflag, CCaptureFlag);

CCaptureFlag::CCaptureFlag()
{

#ifdef GAME_DLL
	m_iszModel = NULL_STRING;
	m_iszHudIcon = NULL_STRING;
	m_iszPaperEffect = NULL_STRING;
	m_iszTrailEffect = NULL_STRING;
	m_flTimeToSetPoisonous = 0.0;
#endif
	
	m_vecResetPositionOffset = { 0, 0, 0 };
	m_bVisibleWhenDisabled = false;
	m_pInitialPlayer = NULL;
	m_pInitialParent = NULL;
}

int CCaptureFlag::GetItemID() const
{
	return OF_ITEM_FLAG;
}

void CCaptureFlag::GetHudIcon(int iTeam, char *pHudIconName, int pHudIconNameSize )
{
	const char *pHudIcon;
	if (m_szHudIcon == '\0')
	{
		pHudIcon = OF_FLAG_HUDICON;
	}
	else
	{
		pHudIcon = m_szHudIcon.Get();
	}

	const char *pTeamName = "red";
	if (iTeam == OF_TEAM_BLUE) pTeamName = "blue";

	V_snprintf(pHudIconName, pHudIconNameSize, "%s_%s", pHudIcon, pTeamName);
}

const char *CCaptureFlag::GetModel()
{
	if (m_szModel.Get() != '\0')
	{
		if (g_pFullFileSystem->FileExists(m_szModel.Get(), "GAME"))
		{
			return m_szModel.Get();
		}
	}

	return OF_FLAG_MODEL;
}

const char *CCaptureFlag::GetPaperEffect()
{
	if (m_szPaperEffect.Get() != '\0')
	{
		return m_szPaperEffect.Get();
	}
	return OF_FLAG_PAPEREFFECT;
}

void CCaptureFlag::GetTrailEffect(int iTeam, char *szTrailEffectName, int iTrailEffectNameSize)
{
	const char *pFlagTrail = "flagtrail";
	if (m_szTrailEffect.Get() != '\0')
	{
		pFlagTrail = m_szTrailEffect.Get();
	}

	const char *pTeamName = "red";
	if (iTeam == OF_TEAM_BLUE) pTeamName = "blu";

	V_snprintf(szTrailEffectName, iTrailEffectNameSize, "effects/%s_%s.vmt", pFlagTrail, pTeamName);
}

void CCaptureFlag::Precache()
{
	PrecacheModel(GetModel());
	PrecacheScriptSound("CaptureFlag.FlagSpawn");
	PrecacheScriptSound("CaptureFlag.EnemyStolen");
	PrecacheScriptSound("CaptureFlag.EnemyDropped");
	PrecacheScriptSound("CaptureFlag.EnemyCaptured");
	PrecacheScriptSound("CaptureFlag.EnemyReturned");
	PrecacheScriptSound("CaptureFlag.TeamStolen");
	PrecacheScriptSound("CaptureFlag.TeamDropped");
	PrecacheScriptSound("CaptureFlag.TeamCaptured");
	PrecacheScriptSound("CaptureFlag.TeamReturned");
	PrecacheScriptSound("AttackDefend.Captured");
	PrecacheScriptSound("AttackDefend.EnemyStolen");
	PrecacheScriptSound("AttackDefend.EnemyDropped");
	PrecacheScriptSound("AttackDefend.EnemyCaptured");
	PrecacheScriptSound("AttackDefend.EnemyReturned");
	PrecacheScriptSound("AttackDefend.TeamStolen");
	PrecacheScriptSound("AttackDefend.TeamDropped");
	PrecacheScriptSound("AttackDefend.TeamCaptured");
	PrecacheScriptSound("AttackDefend.TeamReturned");
	PrecacheScriptSound("MVM.AttackDefend.EnemyStolen");
	PrecacheScriptSound("MVM.AttackDefend.EnemyDropped");
	PrecacheScriptSound("MVM.AttackDefend.EnemyCaptured");
	PrecacheScriptSound("MVM.AttackDefend.EnemyReturned");
	PrecacheScriptSound("Invade.EnemyStolen");
	PrecacheScriptSound("Invade.EnemyDropped");
	PrecacheScriptSound("Invade.EnemyCaptured");
	PrecacheScriptSound("Invade.TeamStolen");
	PrecacheScriptSound("Invade.TeamDropped");
	PrecacheScriptSound("Invade.TeamCaptured");
	PrecacheScriptSound("Invade.FlagReturned");
	PrecacheScriptSound("Resource.FlagSpawn");
	PrecacheScriptSound("Announcer.SD_TheirTeamHasFlag");
	PrecacheScriptSound("Announcer.SD_TheirTeamDroppedFlag");
	PrecacheScriptSound("Announcer.SD_TheirTeamCapped");
	PrecacheScriptSound("Announcer.SD_OurTeamHasFlag");
	PrecacheScriptSound("Announcer.SD_OurTeamDroppedFlag");
	PrecacheScriptSound("Announcer.SD_OurTeamCapped");
	PrecacheScriptSound("Announcer.SD_FlagReturned");
	PrecacheScriptSound("Announcer.SD_Event_TheirTeamHasFlag");
	PrecacheScriptSound("Announcer.SD_Event_TheirTeamDroppedFlag");
	PrecacheScriptSound("Announcer.SD_Event_OurTeamHasFlag");
	PrecacheScriptSound("Announcer.SD_Event_OurTeamDroppedFlag");
	PrecacheScriptSound("Announcer.SD_Event_FlagReturned");
	PrecacheScriptSound("Announcer.SD_Event_FlagNags");
	PrecacheScriptSound("Announcer.SD_Event_CappedRed");
	PrecacheScriptSound("Announcer.SD_Event_CappedBlu");
	PrecacheScriptSound("RD.EnemyStolen");
	PrecacheScriptSound("RD.EnemyDropped");
	PrecacheScriptSound("RD.EnemyCaptured");
	PrecacheScriptSound("RD.EnemyReturned");
	PrecacheScriptSound("RD.TeamStolen");
	PrecacheScriptSound("RD.TeamDropped");
	PrecacheScriptSound("RD.TeamCaptured");
	PrecacheScriptSound("RD.TeamReturned");
	PrecacheScriptSound("CaptureFlag.TeamCapturedExcited");

	PrecacheParticleSystem(GetPaperEffect());

	char szTrailEffect[MAX_PATH];
	GetTrailEffect(OF_TEAM_BLUE,szTrailEffect,sizeof(szTrailEffect));
	PrecacheModel(szTrailEffect);

	GetTrailEffect(OF_TEAM_RED, szTrailEffect, sizeof(szTrailEffect));
	PrecacheModel(szTrailEffect);
}

void CCaptureFlag::Spawn()
{
	#ifdef GAME_DLL
		V_strncpy(m_szModel.GetForModify(), STRING(m_iszModel), MAX_PATH);
		V_strncpy(m_szHudIcon.GetForModify(), STRING(m_iszHudIcon), MAX_PATH);
		V_strncpy(m_szPaperEffect.GetForModify(), STRING(m_iszPaperEffect), MAX_PATH);
		V_strncpy(m_szTrailEffect.GetForModify(), STRING(m_iszTrailEffect), MAX_PATH);
	#endif

	// hmm, likely precache, as what else would you want to do before you set a new model?
	//(**(code **)(this->field_0x0 + 0x88))(this);
	Precache();

	SetModel(GetModel());
	SetSolid(SOLID_BBOX);
	SetSolidFlags(FSOLID_TRIGGER | FSOLID_NOT_SOLID);
	SetSize(vec3_origin, vec3_origin);
	CollisionProp()->UseTriggerBounds(true, 24.0);

	#ifdef GAME_DLL
		m_bDisabled = m_bStartDisabled;
		m_bStartDisabled = false;
		m_bRemoveFlagTrailInstantly = false;
		m_flTimeToSetPoisonous = 0.0;
		//field_0x464 = 0; // unused?
	#else
		m_bDisabled = false;
	#endif

	BaseClass::Spawn();
	SetCollisionBounds(Vector(-19.5, -22.5, -6.5), Vector(19.5, 22.5, 6.5));

	#ifdef GAME_DLL
		m_vecResetPosition = GetAbsOrigin();
		m_vecResetAngles = GetAbsAngles();

		CBaseEntity *pParent = GetMoveParent();
		if (pParent)
		{
			m_pInitialParent = pParent; // field_0xc04
		}

		SetFlagStatus(OF_FLAGSTATUS_NONE);
		//ResetFlagReturnTime();
		//ResetFlagNeutralTime();

		m_pInitialPlayer = NULL;
		field_0xbfc = true;

		// networked (field_0x684)
		m_hPrevOwner = NULL;

		m_bCaptured = false;

		///////////
		// mvm stuff going on here, ignored
		///////////

		///////////
		// even more weird things..
		///////////
	#endif

	SetDisabled(m_bDisabled);
}

void CCaptureFlag::SetFlagStatus(int iFlagStatus, CBasePlayer *pPlayer)
{
	#ifdef GAME_DLL
	MDLCACHE_CRITICAL_SECTION();
	#endif

	if (m_nFlagStatus != iFlagStatus)
	{
		m_nFlagStatus = iFlagStatus;

		IGameEvent *pEvent = gameeventmanager->CreateEvent("flagstatus_update");
		if (pEvent)
		{
			#ifdef GAME_DLL
			pEvent->SetInt("userid", pPlayer->GetUserID());
			pEvent->SetInt("entindex", entindex());
			#endif
			gameeventmanager->FireEvent(pEvent);
		}
	}

	#ifdef GAME_DLL
	switch (iFlagStatus)
	{
	case OF_FLAGSTATUS_NONE:
	case OF_FLAGSTATUS_DROPPED:
		ResetSequence(LookupSequence("spin"));
		break;
	case OF_FLAGSTATUS_TAKEN:
		ResetSequence(LookupSequence("idle"));
		break;
	}
	#endif
}

bool CCaptureFlag::IsDisabled()
{
	return m_bDisabled;
}

void CCaptureFlag::SetVisibleWhenDisabled(bool bIsVisible)
{
	m_bVisibleWhenDisabled = bIsVisible;
	SetDisabled(IsDisabled());
}

void CCaptureFlag::SetDisabled(bool bDisable)
{
	m_bDisabled.Set(bDisable);

	if (bDisable)
	{
		if (m_bVisibleWhenDisabled)
		{
			// somehow ghidra determined setrendermode's first parameter was a bool, amazing..
			// ive already fixed it though
			//C_BaseEntity::SetRenderMode((RenderMode_t)this, 4);
			SetRenderMode(kRenderTransAlpha);

			//local_10 = CONCAT13(0xb4, (int3)this->field_0x4c); // 180
			//if (this->field_0x4c != 180) //local_10
			//{
			//	this->field_0x4c = 180;
			//}
			SetRenderColorA(180);

			//*(byte *)&this->field_0x6c = *(byte *)&this->field_0x6c & 0xdf;
			//(**(code **)(this->field_0x0 + 0x224))(this);
			RemoveEffects(EF_NODRAW);
		}
		else
		{
			AddEffects(EF_NODRAW);
		}
		//this->field_0x3c = 0;
		//this->field_0x38 = 0;
		SetTouch(NULL);

		//C_BaseEntity::ThinkSet((FuncDef5 *)this, 0.0, (char *)0x0);
		SetThink(NULL);
	}
	else
	{
		//*(byte *)&this->field_0x6c = *(byte *)&this->field_0x6c & 0xdf;
		//(**(code **)(this->field_0x0 + 0x224))(this);
		RemoveEffects(EF_NODRAW);

		//C_BaseEntity::SetRenderMode((RenderMode_t)this, 0);
		SetRenderMode(kRenderNormal);

		//local_10 = CONCAT13(0xff, (int3)this->field_0x4c); // 255
		//if (this->field_0x4c != local_10){ this->field_0x4c = local_10; }
		SetRenderColorA(255);

		//this->field_0x38 = FlagTouch;
		//this->field_0x3c = 0;
		SetTouch(&CCaptureFlag::FlagTouch);

		//C_BaseEntity::ThinkSet((FuncDef5 *)this, 9.76705e-43, (char *)0x0);
		SetThink(&CCaptureFlag::Think);
		//C_BaseEntity::SetNextThink
		//	((C_BaseEntity *)this, *(float *)(*(int *)PTR__gpGlobals_00f8a098 + 0xc), (char *)0x0);
		SetNextThink(gpGlobals->curtime);
	}

	#ifdef CLIENT_DLL
	//UpdateGlowEffect();
	#endif
}

void CCaptureFlag::UpdateOnRemove()
{
	// there's some particle stuff going on here

	COFPlayer *pPlayer = dynamic_cast<COFPlayer*>(GetOwnerEntity());
	if (pPlayer)
	{
		pPlayer->SetItem(NULL);
	}
	BaseClass::UpdateOnRemove();
}

#ifdef GAME_DLL
void CCaptureFlag::PlaySound(IRecipientFilter& filter, char const* pSoundName, int param_3)
{
	// there's some mvm and sd stuff going on here, ill do the sd stuff later

	EmitSound(filter, entindex(), pSoundName);
}

void CCaptureFlag::Activate()
{
	BaseClass::Activate();

	field_0xae4 = GetTeamNumber();

	int iSkin = 2;
	if (GetTeamNumber() != 0) { iSkin = (GetTeamNumber() - 2); }
	m_nSkin = iSkin;
}

// figure out the flag types and add them in of_shareddefs!
CCaptureFlag *CCaptureFlag::Create(const Vector &vecOrigin, const char *modelname, EOFFlagType flagType)
{
	CCaptureFlag *pFlag = static_cast<CCaptureFlag*>(CBaseEntity::CreateNoSpawn("item_teamflag", vecOrigin, vec3_angle, NULL));
	
	pFlag->m_iszModel = MAKE_STRING(modelname);
	pFlag->m_nType = flagType;

	if (flagType == 6)
	{
		pFlag->m_nUseTrailEffect = 0;
	}

	DispatchSpawn(pFlag);
	return pFlag;
}
#endif

void CCaptureFlag::Reset()
{
#ifdef GAME_DLL
	m_bRemoveFlagTrailInstantly = true;
	RemoveFlagTrail();

	if (m_nType == TF_FLAGTYPE_PLAYERDESTRUCTION)
	{
		UTIL_Remove(this);
		return;
	}

	// fifth gamemode is robot dectruction
	if ((m_nType == TF_FLAGTYPE_ROBOTDESTRUCTION) && (m_bDisabled == false))
	{

	}

	if (m_pInitialParent)
	{
		SetAbsOrigin(m_pInitialParent->GetAbsOrigin() + m_vecResetPositionOffset);
		SetParent(m_pInitialParent);
	}
	else
	{
		SetAbsOrigin(m_vecResetPosition);
		SetParent(NULL);
	}

	SetAbsAngles(m_vecResetAngles);
	SetFlagStatus(OF_FLAGSTATUS_NONE);

	//ResetFlagReturnTime();
	//ResetFlagNeutralTime();

	m_pInitialPlayer = NULL;
	field_0xbfc = true;

	m_hPrevOwner = NULL;
	m_flTimeToSetPoisonous = 0.0;

	if (m_nType - 3 < 2)
	{

	}

	SetMoveType(MOVETYPE_NONE);

	// mvm stuff here, ignored
#endif
}

#ifdef GAME_DLL
void CCaptureFlag::Think()
{
	if (m_bDisabled) return;

	if (OFGameRules()->FlagsMayBeCapped())
	{
		if (m_bCaptured)
		{
			m_bCaptured = false;
			SetTouch(&CCaptureFlag::FlagTouch);
		}

		if (m_nFlagStatus == 1)
		{

		}
	}

	SetNextThink(gpGlobals->curtime + 0.25);
}
#endif

void CCaptureFlag::FlagTouch(CBaseEntity *param_1)
{
	if ((!m_bDisabled) && (m_nFlagStatus != OF_FLAGSTATUS_TAKEN) && param_1->IsPlayer() && param_1->IsAlive())
	{
		if (m_hPrevOwner.Get() && (m_hPrevOwner.Get() == param_1) && (field_0xbfc == false))
		{
			return;
		}

		if (param_1->GetTeamNumber() == GetTeamNumber())
		{
			#ifdef GAME_DLL
				m_OnTouchSameTeam.FireOutput(this, this);
			#endif
			if (m_nType == TF_FLAGTYPE_CTF || m_nType == TF_FLAGTYPE_ROBOTDESTRUCTION)
			{
				//if (*(int *)(DAT_01050328 + 0x30) == 0) return;
				if (m_nFlagStatus < 2) return;
			}
		}

		if (m_nType == TF_FLAGTYPE_UNKNOWN1 || m_nType == TF_FLAGTYPE_UNKNOWN2)
		{
			if (param_1->GetTeamNumber() != GetTeamNumber()) return;
		}

		if ((m_nType == TF_FLAGTYPE_UNKNOWN3 || m_nType == TF_FLAGTYPE_UNKNOWN4) && GetTeamNumber() != TEAM_UNASSIGNED)
		{
			if (param_1->GetTeamNumber() != GetTeamNumber()) return;
		}

		// this is IsInWaitingForPlayers
		// PTR__g_pGameRules_00e340a8 + 900
		COFPlayer *pPlayer = dynamic_cast<COFPlayer*>(param_1);
		if ((pPlayer) && !(OFGameRules()->IsInWaitingForPlayers()) && pPlayer->IsPlayer() && pPlayer->m_Shared.IsAllowedToPickUpFlag())
		{
			//if (!OFGameRules()->0x966)
			//{
				//if (pPlayer->m_Shared.InCond(TF_COND_SELECTED_TO_TELEPORT)) return;

				//if (pPlayer->m_Shared.IsInvulnerable()) return;

				//if (pPlayer->m_Shared.IsStealthed()) return;

				//if (pPlayer->m_Shared.InCond(TF_COND_STEALTHED_BLINK)) return;

				//if (0.25 < (pPlayer->m_Shared.GetPercentInvisible())) return;

				//if (pPlayer->m_Shared.InCond(TF_COND_PHASE)) return;

				//if (pPlayer->m_shared.HasTheFlag(TF_FLAGTYPE_UNKNOWN,0) && !(m_nType == TF_FLAGTYPE_ROBOTDESTRUCTION || m_nType == TF_FLAGTYPE_UNKNOWN6) && !tf_flag_return_on_touch.GetBool()) return;
				
				//if (PointInRespawnRoom(pPlayer,pPlayer->WorldSpaceCenter(),false)) return;

				if (IsDropped())
				{
					if (param_1->GetTeamNumber() == GetTeamNumber() && m_nType == TF_FLAGTYPE_CTF && tf_flag_return_on_touch.GetBool())
					{
						Reset();
						//ResetMessage();
						//CTFGameStats::Event_PlayerReturnedFlag((CTFGameStats *)PTR__CTF_GameStats_00e340b8, (CTFPlayer *)param_1);
						return;
					}
				}
			//}

		Pickup(pPlayer, true);
		}
	}
}

bool CCaptureFlag::IsStolen()
{
	return m_nFlagStatus == OF_FLAGSTATUS_TAKEN;
}

bool CCaptureFlag::IsHome()
{
	return m_nFlagStatus == OF_FLAGSTATUS_NONE;
}

bool CCaptureFlag::IsDropped()
{
	return m_nFlagStatus == OF_FLAGSTATUS_DROPPED;
}

void CCaptureFlag::Pickup(COFPlayer *pPlayer, bool bNoDraw)
{
	if (m_bDisabled != false) return;

	if (!OFGameRules()->FlagsMayBeCapped()) return;

	//if (OFGameRules()->0x966 && pPlayer->HasTheFlag(TF_FLAGTYPE_UNKNOWN, 0)) return;

	#ifdef GAME_DLL
	// ... buuuunch of stuff
	#endif

	BaseClass::Pickup(pPlayer, false);
	//pPlayer->m_Shared.OpenFortress_SetSpeed();

	#ifdef GAME_DLL
	
	int iAttachment = pPlayer->LookupAttachment("flag");
	if (0 < iAttachment)
	{
		SetParent(pPlayer, iAttachment);
		SetLocalOrigin(vec3_origin);
		SetLocalAngles(vec3_angle);
	}

	// the 8th part is very likely checking for the class type
	//if ((pPlayer->0x2114 == 8) && (m_nType != TF_FLAGTYPE_UNKNOWN6))
	//{
	//	if (pPlayer->m_Shared.InCond(TF_COND_DISGUISED) || pPlayer->m_Shared.InCond(TF_COND_DISGUISING))
	//	{
	//		pPlayer->m_Shared.RemoveDisguise();
	//	}
	//}

	m_nSkin = m_nSkin.Get() + 3;
	SetTouch(NULL);
	m_hPrevOwner = pPlayer;
	field_0xbfc = true;

	if (m_pInitialPlayer == NULL)
	{
		if (pPlayer)
		{
			m_pInitialPlayer = pPlayer;
		}
	}

	// OFTODO: not sure what to do about this so... figure it out later
	// there's just a huge portion of nothing from 00194a46 to 00195335 in the server
	// WARNING: Could not recover jumptable at 0x00194a44. Too many branches
	// WARNING: Treating indirect jump as call

	SetFlagStatus(OF_FLAGSTATUS_TAKEN, pPlayer);
	//ResetFlagReturnTime()
	//ResetFlagNeutralTime()

	IGameEvent *pEvent = gameeventmanager->CreateEvent("teamplay_flag_event", false);
	if (pEvent)
	{
		pEvent->SetInt("player", pPlayer->entindex());
		pEvent->SetInt("eventtype", 1);
		pEvent->SetInt("priority", 8);
		pEvent->SetInt("home", (m_nFlagStatus == OF_FLAGSTATUS_NONE));
		pEvent->SetInt("team", GetTeamNumber());
		gameeventmanager->FireEvent(pEvent);
	}

	pPlayer->SpeakConceptIfAllowed(MP_CONCEPT_FLAGPICKUP);

	m_outputOnPickUp.FireOutput(this, this);

	switch (pPlayer->GetTeamNumber())
	{
	case OF_TEAM_BLUE:
		m_outputOnPickUpTeam2.FireOutput(this, this);
		break;
	case OF_TEAM_RED:
		m_outputOnPickUpTeam1.FireOutput(this, this);
		break;
	default:
		break;
	}

	// take a look at this later - cherry
	// line 196-204
	//if (m_pReturnIcon)
	//{
	//}

	StartFlagTrail();
	//HandleFlagPickedUpInDetectionZone(pPlayer);

	// there's some bonus mvm stuff beyond here, so ignore that

	#endif
}

#ifdef GAME_DLL

int CCaptureFlag::GetMaxReturnTime()
{
	return m_nReturnTime;
	// trimmed playerdestruction + robotdestruction
}

void CCaptureFlag::StartFlagTrail()
{
	if ((m_nUseTrailEffect == 0) || (m_nUseTrailEffect == 2)) return;

	if (m_pFlagTrail) return;

	COFPlayer *pPlayer = ToOFPlayer(m_hPrevOwner);

	if (pPlayer)
	{
		int iTeam = pPlayer->GetTeamNumber();

		char szTrailEffectName[256];
		GetTrailEffect(iTeam, szTrailEffectName, sizeof(szTrailEffectName));

		CSpriteTrail *pTrail = CSpriteTrail::SpriteTrailCreate(szTrailEffectName, GetAbsOrigin(), true);
		pTrail->FollowEntity(this, true);
		pTrail->SetTransparency(kRenderTransAlpha, 255, 255, 255, 96, kRenderFxNone);
		pTrail->SetStartWidth(32.0);
		pTrail->SetTextureResolution(0.01041667);
		pTrail->SetLifeTime(0.7);
		pTrail->TurnOn();
		pTrail->SetTransmit(false);

		m_pFlagTrail = pTrail;
	}
}

void CCaptureFlag::RemoveFlagTrail()
{
	if (m_pFlagTrail)
	{
		if ((m_fFlagTrailBrightness <= 0.0) || m_bRemoveFlagTrailInstantly)
		{
			UTIL_Remove(m_pFlagTrail);
			m_fFlagTrailBrightness = 1.0;
		}
		else
		{
			CSpriteTrail *pTrail = dynamic_cast<CSpriteTrail*>(m_pFlagTrail.Get());

			pTrail->SetBrightness(int(m_fFlagTrailBrightness * 96.0));

			m_fFlagTrailBrightness = m_fFlagTrailBrightness - 0.1;
			SetThink(&CCaptureFlag::RemoveFlagTrail);
		}
		m_bRemoveFlagTrailInstantly = false;
	}
}

// OFSTATUS: INCOMPLETE
// ive only done the first case which is just normal ctf which still needs work
void CCaptureFlag::Capture(COFPlayer *pPlayer, int param_2)
{
	if (m_bDisabled) return;

	switch (m_nType)
	{
	// i feel like i can safely assume the first flagetype is just normal ctf
	case TF_FLAGTYPE_CTF:

		// give this a proper name later
		bool bCheck = false;

		if (tf_flag_caps_per_round.GetInt() >= 1)
		{
			int iFlagsCaptured = OFTeamMgr()->GetFlagCaptures(pPlayer->GetTeamNumber());

			if ((iFlagsCaptured < 0) || (1 < tf_flag_caps_per_round.GetInt() - iFlagsCaptured))
			{
				bCheck = true;
			}
		}

		if (bCheck)
		{
			for (int iTeam = FIRST_GAME_TEAM; iTeam < OF_TEAM_COUNT; iTeam++)
			{
				if (pPlayer->GetTeamNumber() == iTeam)
				{
					// note: 0x961 for CTFGameRules is a check for powerup mode!
					//if (OFGameRules()->0x961)
					CTeamRecipientFilter filter(iTeam, true);
					PlaySound(filter, "CaptureFlag.TeamCaptured");
					//OFGameRules()->SendHudNotification(filter, 7, false);
				}
				else
				{
					CTeamRecipientFilter filter(iTeam, true);
					PlaySound(filter, "CaptureFlag.EnemyCaptured");
					//OFGameRules()->SendHudNotification(filter, 3, false);
				}
			}

			if (OFGameRules())
			{
				//OFGameRules()->HandleCTFCaptureBonus(pPlayer->GetTeamNumber());
			}
		}

		// some currency(???) stuff happens here
		// i dont think that was 2008 tf2.. so ill ignore it

		if (m_pInitialPlayer && m_pInitialPlayer != pPlayer)
		{
			//CTF_GameStats->Event_PlayerCapturedPoint(m_pInitialPlayer);
			m_pInitialPlayer = NULL;
		}

		if (tf_flag_caps_per_round.GetInt() >= 1)
		{
			OFTeamMgr()->IncrementFlagCaptures(pPlayer->GetTeamNumber());
		}
		else
		{
			OFTeamMgr()->AddTeamScore(pPlayer->GetTeamNumber(), 1);
		}

		break;
	} // end switch statement

	//if ((0.0 < m_flTimeToSetPoisonous) && (m_flTimeToSetPoisonous < gpGlobals->curtime))
	//{
	//	pPlayer->m_Shared.RemoveCond(TF_COND_MARKEDFORDEATH,false);
	//}

	IGameEvent *pEvent = gameeventmanager->CreateEvent("teamplay_flag_event", false);
	if (pEvent)
	{
		pEvent->SetInt("player", pPlayer->entindex());
		pEvent->SetInt("eventtype", 2);
		pEvent->SetInt("priority", 9);
		pEvent->SetInt("team", GetTeamNumber());
		gameeventmanager->FireEvent(pEvent);
	}

	SetFlagStatus(OF_FLAGSTATUS_NONE);
	//ResetFlagReturnTime();
	//ResetFlagNeutralTime();
	m_bRemoveFlagTrailInstantly = true;
	RemoveFlagTrail();

	int iSkin = 2;
	if (GetTeamNumber() != TEAM_UNASSIGNED) iSkin = GetTeamNumber() - 2;
	m_nSkin = iSkin;

	//HandleFlagCapturedInDetectionZone(pPlayer);
	//HandleFlagDroppedInDetectionZone(pPlayer);

	BaseClass::Drop(pPlayer, true, false, true);
	Reset();

	//pPlayer->TeamFortress_SetSpeed();

	// CTFGameRules field_0xd9c is a check for halloween scenario
	pPlayer->SpeakConceptIfAllowed(MP_CONCEPT_FLAGCAPTURED);

	m_outputOnCapture.FireOutput(this, this);

	switch (pPlayer->GetTeamNumber())
	{
	case OF_TEAM_RED:
		m_OnCapTeam1.FireOutput(this, this);
		break;
	case OF_TEAM_BLUE:
		m_OnCapTeam2.FireOutput(this, this);
		break;
	}

	m_bCaptured = true;

	SetNextThink(gpGlobals->curtime + 0.25);

	if (OFGameRules()->InStalemate())
	{
		COFTeam *pTeam = pPlayer->GetOFTeam();
		if (pTeam && tf_flag_caps_per_round.GetInt() > pTeam->GetFlagCaptures())
		{
			pTeam->SetFlagCaptures(tf_flag_caps_per_round.GetInt());
		}
	}
}
#endif

void CCaptureFlag::Drop(COFPlayer *pPlayer, bool bNoDraw, bool param_3, bool param_4)
{
	if (m_bDisabled) return;

	BaseClass::Drop(pPlayer, bNoDraw);
	//pPlayer->TeamFortress_SetSpeed();

	#ifdef GAME_DLL

	if (param_3)
	{
		field_0xbfc = false;
		field_0xae8 = gpGlobals->curtime + 3.0;
	}

	// theres so much stuff going on here, nav meshes, engine stuff,
	// i wont even try to make sense of it, we can do it later

	// oh yeah and this again:
	// WARNING: Could not recover jumptable at 0x00197e22. Too many branches
	// WARNING: Treating indirect jump as call
	// from 00197e24 to 001985b6

	//if (0.0 < m_flTimeToSetPoisonous && m_flTimeToSetPoisonous < gpGlobals->curtime)
	//{
	//	pPlayer->m_Shared.RemoveCond(TF_COND_MARKEDFORDEATH);
	//}

	m_nSkin = m_nSkin - 3;

	RemoveFlagTrail();

	//SetFlagReturnIn(GetMaxReturnTime(), GetMaxReturnTime());

	SetAbsAngles(m_vecResetAngles);
	SetTouch(&CCaptureFlag::FlagTouch);
	SetFlagStatus(OF_FLAGSTATUS_DROPPED, NULL);

	m_outputOnDrop.FireOutput(this, this);

	// 0x966 of CTFGameRules is a check for mvm mode
	if (GetMaxReturnTime() < 600)
	{
		//CreateReturnIcon();
	}

	//if (PointInRespawnFlagZone(GetAbsOrigin()))
	//{
		//Reset();
		//ResetMessage();
	//}

	//HandleFlagDroppedInDetectionZone(pPlayer);

	// mvm stuff here, ignored

	#endif
}

/*
bool PointInRespawnFlagZone(const Vector &vector)
{
	CBaseEntity *pEntity = gEntList.FindEntityByClassname(NULL, "func_respawnflag");
	while (pEntity != NULL)
	{
		CFuncRespawnFlagZone *pFlagZone = dynamic_cast<CFuncRespawnFlagZone*>(pEntity);

		if (pFlagZone && !pFlagZone->field_0x3e0 && pFlagZone->PointIsWithin(vector))
		{
			return true;
		}

		pEntity = gEntList.FindEntityByClassname(pEntity, "func_respawnflag");
	}

	return false;
}
*/

#ifdef GAME_DLL
// OFSTATUS: COMPLETE
void CCaptureFlag::CreateReturnIcon()
{
	if (!m_pReturnIcon) return;

	Vector vecOrigin = GetAbsOrigin() + Vector(0, 0, cl_flag_return_height.GetInt());
	CBaseEntity *pIcon = CBaseEntity::Create("item_teamflag_return_icon", vecOrigin, vec3_angle, this);

	if (pIcon)
	{
		m_pReturnIcon = pIcon;
		pIcon->SetParent(this);
	}
}

void CCaptureFlag::SetFlagReturnIn(float fReturnTime)
{
	m_flResetTime = gpGlobals->curtime + fReturnTime;
	m_flMaxResetTime = fReturnTime;
}

void CCaptureFlag::InputEnable(inputdata_t &inputdata)
{
	SetDisabled(false);
}

void CCaptureFlag::InputDisable(inputdata_t &inputdata)
{
	SetDisabled(true);
}

// OFSTATUS: COMPLETE
void CCaptureFlag::InputSetReturnTime(inputdata_t &inputdata)
{
	int iReturnTime = inputdata.value.Int();

	if (IsDropped())
	{
		SetFlagReturnIn(float(iReturnTime));
	}
}

#endif