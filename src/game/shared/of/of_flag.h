// ========= Copyright Open Fortress Developers, CC-BY-NC-SA ============
// Purpose: Implementation of CCaptureFlag
// Author(s): Cherry!
//

#pragma once

#include "of_item.h"
#include "of_shareddefs.h"

#ifdef CLIENT_DLL
	#define CCaptureFlag C_CaptureFlag
#endif

DECLARE_AUTO_LIST(ICaptureFlagAutoList);

class CCaptureFlag : public COFItem, public ICaptureFlagAutoList
{
public:
	DECLARE_CLASS(CCaptureFlag,COFItem);
	DECLARE_NETWORKCLASS();
	DECLARE_DATADESC();

	CCaptureFlag();

	int GetItemID() const;
	void GetHudIcon(int param_1, char *param_2, int param_3);

	const char *GetModel();
	const char *GetPaperEffect();
	void GetTrailEffect(int iTeam, char *pTrailEffectName, int iTrailEffectNameSize);

	void Precache();
	void Spawn();

	void SetFlagStatus(int iFlagStatus, CBasePlayer *pPlayer = NULL);

	bool IsDisabled();
	void SetVisibleWhenDisabled(bool bIsVisible);
	void SetDisabled(bool bDisabled);
	void UpdateOnRemove();
	void Reset();
#ifdef GAME_DLL
	void PlaySound(IRecipientFilter& filter, const char *pSoundName, int param_3 = -2);
	void Activate();
	static CCaptureFlag *Create(const Vector &vecOrigin, const char *modelname, EOFFlagType flagType);

	void Think();
#endif
	void FlagTouch(CBaseEntity *param_1);
	bool IsStolen();
	bool IsHome();
	bool IsDropped();
	void Pickup(COFPlayer *pPlayer, bool bNoDraw);
#ifdef GAME_DLL
	int GetMaxReturnTime();
	void StartFlagTrail();
	void RemoveFlagTrail();
	void Capture(COFPlayer *pPlayer, int param_2);
	void CreateReturnIcon();
	void SetFlagReturnIn(float fReturnTime);
	bool IsCaptured() { return m_bCaptured; };

	void InputEnable(inputdata_t &inputdata);
	void InputDisable(inputdata_t &inputdata);
	void InputSetReturnTime(inputdata_t &inputdata);
#endif
	void Drop(COFPlayer *pPlayer, bool bNoDraw, bool param_3 = false, bool param_4 = true);
	int GetFlagType() { return m_nType; };
private:

	int m_nReturnTime;
	int m_nNeutralType;
	int m_nScoringType;
	#ifdef GAME_DLL
		string_t m_iszModel;
		string_t m_iszHudIcon;
		string_t m_iszPaperEffect;
		string_t m_iszTrailEffect;
	#endif

	CNetworkVar(bool, m_bDisabled);
	CNetworkVar(bool, m_bVisibleWhenDisabled);
	CNetworkVar(int, m_nType);
	CNetworkVar(int, m_nFlagStatus);
	CNetworkVar(float, m_flResetTime);
	CNetworkVar(float, m_flNeutralTime);
	CNetworkVar(float, m_flMaxResetTime);
	CNetworkHandle(CBaseEntity, m_hPrevOwner);
	CNetworkString(m_szModel, MAX_PATH);
	CNetworkString(m_szHudIcon, MAX_PATH);
	CNetworkString(m_szPaperEffect, MAX_PATH);
	CNetworkString(m_szTrailEffect, MAX_PATH);
	CNetworkVar(int, m_nUseTrailEffect);
	//CNetworkHandle(m_nPointValue);
	//CNetworkVar(float, m_flAutoCapTime);
	//CNetworkVar(bool, m_bGlowEnabled);
	CNetworkVar(float, m_flTimeToSetPoisonous);

	Vector m_vecResetPosition;
	QAngle m_vecResetAngles;
	Vector m_vecResetPositionOffset;

	COFPlayer *m_pInitialPlayer;

	CBaseEntity *m_pInitialParent;

	int field_0xae4;
	bool field_0xbfc;

	#ifdef GAME_DLL

	COutputEvent m_outputOnReturn;
	COutputEvent m_OnTouchSameTeam;
	COutputEvent m_outputOnPickUp;
	COutputEvent m_outputOnPickUpTeam1;
	COutputEvent m_outputOnPickUpTeam2;
	COutputEvent m_outputOnDrop;
	COutputEvent m_outputOnCapture;
	// rename these to red and blue maybe?
	COutputEvent m_OnCapTeam1;
	COutputEvent m_OnCapTeam2;

	EHANDLE m_pFlagTrail;

	float m_fFlagTrailBrightness;
	bool m_bRemoveFlagTrailInstantly;

	bool m_bCaptured;

	#endif

	float field_0xae8;
	CBaseEntity *m_pReturnIcon;
};