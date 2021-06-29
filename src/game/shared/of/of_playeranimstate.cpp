// ========= Copyright Open Fortress Developers, CC-BY-NC-SA ============
// Purpose: 
// Author(s): Nopey, Cherry!
//

#include "cbase.h"
#include "of_shareddefs.h"
#include "of_playeranimstate.h"
#include "tier0/vprof.h"
#include "datacache/imdlcache.h"
#include "of_weapon_base.h"

#ifdef CLIENT_DLL
	#include "c_of_player.h"
#else
	#include "of_player.h"
#endif

COFPlayerAnimState* CreatePlayerAnimState(COFPlayer *pPlayer)
{
	MDLCACHE_CRITICAL_SECTION();

	MultiPlayerMovementData_t movementData;
	movementData.m_flBodyYawRate = 720.0;
	movementData.m_flRunSpeed = 320.0;
	movementData.m_flWalkSpeed = 75.0;
	movementData.m_flSprintSpeed = -1.0;
	//0x44340000 = 720.0
	//0x43a00000 = 320.0
	//0x42960000 = 75.0
	//0xbf800000 = -1.0

	COFPlayerAnimState *pPlayerAnim = new COFPlayerAnimState(pPlayer, movementData);

	//lots of variables here
	pPlayerAnim->InitOF(pPlayer);

	return pPlayerAnim;
}

COFPlayerAnimState::COFPlayerAnimState()
{

}

COFPlayerAnimState::COFPlayerAnimState(CBasePlayer *pPlayer, MultiPlayerMovementData_t &movementData) : CMultiPlayerAnimState(pPlayer, movementData)
{

}

void COFPlayerAnimState::InitOF(COFPlayer *pPlayer)
{
	m_pOFPlayer = pPlayer;
	m_bInAirWalk = false; // field_0xf8
	field_0xfc = 0.0;
}

// decomp is littered with halloween, item test, and more junk
void COFPlayerAnimState::Update(float eyeYaw, float eyePitch)
{
	VPROF("CMultiPlayerAnimState::Update");

	if (!m_pOFPlayer) return;

	CStudioHdr *pStudioHdr = m_pOFPlayer->GetModelPtr();
	if (!pStudioHdr) return;

	// OFTODO: custom model stuff here
	// should re it as apprently it relates to avoiding breaking plugins
	// i wont re it just yet, but i will soon :tm: - cherry

	// Check to see if we should be updating the animation state - dead, ragdolled?
	if (!ShouldUpdateAnimState())
	{
		ClearAnimationState();
		return;
	}

	// Store the eye angles.
	m_flEyeYaw = AngleNormalize(eyeYaw);
	m_flEyePitch = AngleNormalize(eyePitch);

	// Compute the player sequences.
	ComputeSequences(pStudioHdr);

	if (SetupPoseParameters(pStudioHdr))
	{
		// Pose parameter - what direction are the player's legs running in.
		ComputePoseParam_MoveYaw(pStudioHdr);

		// Pose parameter - Torso aiming (up/down).
		ComputePoseParam_AimPitch(pStudioHdr);

		// Pose parameter - Torso aiming (rotation).
		ComputePoseParam_AimYaw(pStudioHdr);
	}

#ifdef CLIENT_DLL
	if (C_BasePlayer::ShouldDrawLocalPlayer())
	{
		GetBasePlayer()->SetPlaybackRate(1.0f);
	}
#endif
}

void COFPlayerAnimState::CleanAnimationState()
{
	m_bInAirWalk = false;
	BaseClass::ClearAnimationState();
}

// Note: ghidra wasnt able to recover the switch statement so i did my
// best in manually finding the addresses that were involved with it - cherry
void COFPlayerAnimState::DoAnimationEvent( PlayerAnimEvent_t event, int nData )
{
	Activity activity = ACT_INVALID;
	bool bInCrouch = ((GetBasePlayer()->GetFlags() & FL_DUCKING) != 0);
	if (bInCrouch && (SelectWeightedSequence(TranslateActivity(ACT_MP_CROUCHWALK)) < 0))
	{
		bInCrouch = false;
	}

	switch (event)
	{
	case PLAYERANIMEVENT_ATTACK_PRIMARY:
		{
			if (!m_pOFPlayer) return;

			// we dont need to check for a bow so just do a check for the rifle
			COFWeaponBase *pWeapon = m_pOFPlayer->GetActiveOFWeapon();
			bool bWeaponIsMiniGun = (pWeapon->GetWeaponID() == OF_WEAPON_MINIGUN);
			bool bWeaponIsSniperRifle = (pWeapon->GetWeaponID() == OF_WEAPON_SNIPERRIFLE);

			if (bWeaponIsMiniGun)
			{
				activity = ACT_MP_ATTACK_STAND_PRIMARYFIRE;

				//ACT_MP_ATTACK_STAND_PRIMARYFIRE
				//ACT_MP_ATTACK_CROUCH_PRIMARYFIRE
				//ACT_MP_ATTACK_SWIM_PRIMARYFIRE

				if (m_bInSwim)
				{
					activity = ACT_MP_ATTACK_SWIM_PRIMARYFIRE;
				}
				else if (bInCrouch)
				{
					activity = ACT_MP_ATTACK_CROUCH_PRIMARYFIRE;
				}

				if (IsGestureSlotPlaying(GESTURE_SLOT_ATTACK_AND_RELOAD, TranslateActivity(activity)))
				{
					RestartGesture(GESTURE_SLOT_ATTACK_AND_RELOAD, activity);
				}
			}
			// OFTODO: DONT FORGET TO UNCOMMENT ONCE CONDITIONS ARE ADDED
			else if (bWeaponIsSniperRifle)// && m_pOFPlayer->InCond(TF_COND_ZOOMED))
			{
				if (GetBasePlayer()->GetFlags() & FL_DUCKING)
				{
					RestartGesture(GESTURE_SLOT_ATTACK_AND_RELOAD, ACT_MP_ATTACK_CROUCH_PRIMARYFIRE_DEPLOYED);
				}
				else
				{
					RestartGesture(GESTURE_SLOT_ATTACK_AND_RELOAD, ACT_MP_ATTACK_STAND_PRIMARYFIRE_DEPLOYED);
				}
				
				field_0xfc = gpGlobals->curtime + 2.0;
			}
			else
			{
				RestartGesture(GESTURE_SLOT_ATTACK_AND_RELOAD, ACT_MP_ATTACK_STAND_PRIMARYFIRE_DEPLOYED);
			}

			break;
		}
	case PLAYERANIMEVENT_ATTACK_SECONDARY:
		{
			if (m_bInSwim)
			{
				RestartGesture(GESTURE_SLOT_ATTACK_AND_RELOAD, ACT_MP_ATTACK_SWIM_SECONDARY);
			}
			else if (GetBasePlayer()->GetFlags() & FL_DUCKING)
			{
				RestartGesture(GESTURE_SLOT_ATTACK_AND_RELOAD, ACT_MP_ATTACK_CROUCH_SECONDARY);
			}
			else
			{
				RestartGesture(GESTURE_SLOT_ATTACK_AND_RELOAD, ACT_MP_ATTACK_STAND_SECONDARY);
			}
			break;
		}
	case PLAYERANIMEVENT_ATTACK_PRE:
		{
			if (!m_pOFPlayer) return;
			
			// m_bFirstJump (0xA0) + 1 = m_bInSwim
			COFWeaponBase *pWeapon = m_pOFPlayer->GetActiveOFWeapon();
			bool bWeaponIsMiniGun = (pWeapon && pWeapon->GetWeaponID() == OF_WEAPON_MINIGUN);

			if (m_bInSwim && bWeaponIsMiniGun)
			{
				activity = ACT_MP_ATTACK_SWIM_PREFIRE;
				//RestartGesture(GESTURE_SLOT_ATTACK_AND_RELOAD, ACT_MP_ATTACK_SWIM_PREFIRE);
			}
			else if (GetBasePlayer()->GetFlags() & FL_DUCKING)
			{
				activity = ACT_MP_ATTACK_CROUCH_PREFIRE;
				//RestartGesture(GESTURE_SLOT_ATTACK_AND_RELOAD, ACT_MP_ATTACK_CROUCH_PREFIRE);
			}
			else
			{
				activity = ACT_MP_ATTACK_STAND_PREFIRE;
			}

			RestartGesture(GESTURE_SLOT_ATTACK_AND_RELOAD, activity, bWeaponIsMiniGun);
			break;
		}
	case PLAYERANIMEVENT_ATTACK_POST:
		{
			if (!m_pOFPlayer) return;

			COFWeaponBase *pWeapon = m_pOFPlayer->GetActiveOFWeapon();
			bool bWeaponIsMiniGun = (pWeapon && pWeapon->GetWeaponID() == OF_WEAPON_MINIGUN);

			//ACT_MP_ATTACK_SWIM_POSTFIRE

			if (m_bInSwim && bWeaponIsMiniGun)
			{
				activity = ACT_MP_ATTACK_SWIM_POSTFIRE;
			}
			else if (GetBasePlayer()->GetFlags() & FL_DUCKING)
			{
				activity = ACT_MP_ATTACK_CROUCH_POSTFIRE;
			}
			else
			{
				activity = ACT_MP_ATTACK_STAND_POSTFIRE;
			}
			
			RestartGesture(GESTURE_SLOT_ATTACK_AND_RELOAD, activity);
			break;
		}
	case PLAYERANIMEVENT_RELOAD:
		{
			if (m_bInAirWalk)
			{
				RestartGesture(GESTURE_SLOT_ATTACK_AND_RELOAD, ACT_MP_RELOAD_AIRWALK);
			}
			else
			{
				BaseClass::DoAnimationEvent(event, nData);
			}

			break;
		}
	case PLAYERANIMEVENT_RELOAD_LOOP:
		{
			if (m_bInAirWalk)
			{
				RestartGesture(GESTURE_SLOT_ATTACK_AND_RELOAD, ACT_MP_RELOAD_AIRWALK_LOOP);
			}
			else
			{
				BaseClass::DoAnimationEvent(event, nData);
			}

			break;
		}
	case PLAYERANIMEVENT_RELOAD_END:
		{
			if (m_bInAirWalk)
			{
				RestartGesture(GESTURE_SLOT_ATTACK_AND_RELOAD, ACT_MP_RELOAD_AIRWALK_END);
			}
			else
			{
				BaseClass::DoAnimationEvent(event, nData);
			}

			break;
		}
	case PLAYERANIMEVENT_DOUBLEJUMP:
		{
			if (!m_pOFPlayer) return;

			if (!m_bJumping)
			{
				m_bJumping = true;
				m_bFirstJumpFrame = true;
				m_flJumpStartTime = gpGlobals->curtime;
				RestartMainSequence();
			}

			// im pretty sure this is a airwalk check as the baseclass doesnt have one, and doing
			// a in-game test: while air walking and then double jumping stops air walking
			m_bInAirWalk = false; // field_0xf8

			// because activitylist is filled with portal, hl2dm, dod.. and other games
			// my guess based on the diffrence of 0x5d5 and 0x439 
			//ACT_MP_DOUBLEJUMP
			//ACT_MP_DOUBLEJUMP_LOSERSTATE
			//ACT_MP_GESTURE_VC_FINGERPOINT_BUILDING

			// OFTODO: CONDITIONS!
			//if (m_pOFPlayer->m_Shared.IsLoser())
			//{
			//	RestartGesture(GESTURE_SLOT_JUMP, ACT_MP_DOUBLEJUMP_LOSERSTATE);
			//}
			//else
			//{
			RestartGesture(GESTURE_SLOT_JUMP, ACT_MP_DOUBLEJUMP);
			//}

			break;
		}
	default:
		{
			BaseClass::DoAnimationEvent(event, nData);
			break;
		}
	}

	COFWeaponBase *pWeapon = m_pOFPlayer->GetActiveOFWeapon();
	if (activity != ACT_INVALID && pWeapon)
	{
		pWeapon->SendWeaponAnim(activity);
	}
}

// littered with econ stuff and cond stuff, all removed
Activity COFPlayerAnimState::TranslateActivity(Activity actDesired)
{
	Activity activity = BaseClass::TranslateActivity(actDesired);
	COFWeaponBase *pWeapon = m_pOFPlayer->GetActiveOFWeapon();

	if (pWeapon)
	{
		activity = pWeapon->ActivityOverride(activity, false);
	}

	return activity;
}

void COFPlayerAnimState::RestartGesture(int iGesture, Activity iGestureActivity, bool bAutoKill)
{
	Activity activity = TranslateActivity(iGestureActivity);
	BaseClass::RestartGesture(iGesture, activity, bAutoKill);
}

bool COFPlayerAnimState::HandleMoving(Activity &idealActivity)
{
	float flSpeed = GetOuterXYSpeed();
	if (MOVING_MINIMUM_SPEED < flSpeed)
	{
		field_0xfc = 0.0;
	}

	//if (m_pOFPlayer->m_Shared.IsLoser())
	//{
	//	return BaseClass::HandleMoving(idealActivity);
	//}

	// OFTODO: conditions :v
	//if (m_pOFPlayer->m_Shared.IsAiming())
	if (false)
	{
		if (MOVING_MINIMUM_SPEED < flSpeed)
		{
			idealActivity = ACT_MP_DEPLOYED;
		}
		else
		{
			idealActivity = ACT_MP_DEPLOYED_IDLE;
		}
	}
	else if (gpGlobals->curtime < field_0xfc)
	{
		idealActivity = ACT_MP_DEPLOYED_IDLE;
	}
	else
	{
		return BaseClass::HandleMoving(idealActivity);
	}

	return true;
}