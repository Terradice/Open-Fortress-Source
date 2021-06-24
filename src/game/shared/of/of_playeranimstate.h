// ========= Copyright Open Fortress Developers, CC-BY-NC-SA ============
// Purpose: 
// Author(s): Nopey, Cherry!
//

#pragma once

#include "iplayeranimstate.h"
#include "base_playeranimstate.h"
#include "Multiplayer/multiplayer_animstate.h"

#ifdef CLIENT_DLL
	class C_OFPlayer;
	#define COFPlayer C_OFPlayer
#else
	class COFPlayer;
#endif

class COFPlayerAnimState : public CMultiPlayerAnimState
{
public:
	DECLARE_CLASS(COFPlayerAnimState, CMultiPlayerAnimState);
	
	COFPlayerAnimState();
	COFPlayerAnimState(CBasePlayer *pPlayer, MultiPlayerMovementData_t &movementData);

	void InitOF(COFPlayer *pPlayer);
	void Update(float eyeYaw, float eyePitch);
	void DoAnimationEvent( PlayerAnimEvent_t event, int nData = 0 ); // This is called by both the client and the server in the same way to trigger events for players
	Activity TranslateActivity(Activity actDesired);
	void RestartGesture(int iGesture, Activity iGestureActivity, bool bAutoKill = true);

private:

	COFPlayer *m_pOFPlayer;
	float field_0xfc;
	bool m_bInAirWalk; // field_0xf8
};

COFPlayerAnimState* CreatePlayerAnimState( COFPlayer *pPlayer );
