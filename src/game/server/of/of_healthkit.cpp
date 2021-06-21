// ========= Copyright Open Fortress Developers, CC-BY-NC-SA ============
// Purpose: Implementation of CHealthKit
// Author(s): Cherry!
//

#include "cbase.h"
#include "of_shareddefs.h"
#include "of_healthkit.h"

IMPLEMENT_AUTO_LIST(IHealthKitAutoList);

LINK_ENTITY_TO_CLASS(item_healthkit_full, CHealthKit);
LINK_ENTITY_TO_CLASS(item_healthkit_medium, CHealthKitMedium);
LINK_ENTITY_TO_CLASS(item_healthkit_small, CHealthKitSmall);

void CHealthKit::Spawn()
{
	BaseClass::Spawn();
}

void CHealthKit::Precache()
{
	PrecacheScriptSound("HealthKit.Touch");
	BaseClass::Precache();
}

// this is full of econ, mannpower, and more bloat
bool CHealthKit::MyTouch(CBasePlayer *pPlayer)
{
	bool bPackUsed = false;

	if (ValidTouch(pPlayer))
	{
		float fHealth = ceil(pPlayer->GetMaxHealth() * PackRatios[GetPowerupSize()]);
		int iHealthGiven = pPlayer->TakeHealth(fHealth, DMG_GENERIC);

		IGameEvent *pEvent = gameeventmanager->CreateEvent("player_healed");
		if (iHealthGiven > 0)
		{
			if (pEvent)
			{
				pEvent->SetInt("priority", 1);
				pEvent->SetInt("patient", pPlayer->GetUserID());
				pEvent->SetInt("healer", pPlayer->GetUserID());
				pEvent->SetInt("amount", iHealthGiven);
				gameeventmanager->FireEvent(pEvent);
			}

			CSingleUserRecipientFilter user(pPlayer);
			user.MakeReliable();
			EmitSound(user, entindex(), "HealthKit.Touch");

			bPackUsed = true;
		}
	}

	return bPackUsed;
}