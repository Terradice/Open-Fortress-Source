// ========= Copyright Open Fortress Developers, CC-BY-NC-SA ============
// Purpose: Implementation of CAmmoPack
// Author(s): Cherry!
//

#include "cbase.h"
#include "of_shareddefs.h"
#include "of_ammopack.h"
#include "of_player.h"

LINK_ENTITY_TO_CLASS(item_ammopack_full, CAmmoPack);
LINK_ENTITY_TO_CLASS(item_ammopack_medium, CAmmoPackMedium);
LINK_ENTITY_TO_CLASS(item_ammopack_small, CAmmoPackSmall);

void CAmmoPack::Spawn()
{
	BaseClass::Spawn();
}

void CAmmoPack::Precache()
{
	PrecacheScriptSound("AmmoPack.Touch");
	BaseClass::Precache();
}

bool CAmmoPack::MyTouch(CBasePlayer *pPlayer)
{
	bool bPackUsed = false;

	COFPlayer *pOFPlayer = ToOFPlayer(pPlayer);
	if (ValidTouch(pPlayer) && pOFPlayer)
	{
		// i didnt wanna add CTFPlayer::GetMaxAmmo as it contained what i presume a bunch of junk
		// so here's a much easier way

		// Primary Ammo
		int iMaxPrimaryAmmo = pOFPlayer->m_Class.GetClassData()->m_iMaxAmmo[OF_AMMO_PRIMARY];
		int iPrimaryAmmoGiven = pOFPlayer->GiveAmmo(ceil(iMaxPrimaryAmmo * PackRatios[GetPowerupSize()]), OF_AMMO_PRIMARY);

		// Secondary Ammo
		int iMaxSecondaryAmmo = pOFPlayer->m_Class.GetClassData()->m_iMaxAmmo[OF_AMMO_SECONDARY];
		int iSecondaryAmmoGiven = pOFPlayer->GiveAmmo(ceil(iMaxSecondaryAmmo * PackRatios[GetPowerupSize()]), OF_AMMO_SECONDARY);

		// Metal
		int iMaxMetal = pOFPlayer->m_Class.GetClassData()->m_iMaxAmmo[OF_AMMO_METAL];
		int iMetalGiven = pOFPlayer->GiveAmmo(ceil(iMaxMetal * PackRatios[GetPowerupSize()]), OF_AMMO_METAL);

		//int iCloakGiven = pOFPlayer->m_Shared.AddToSpyCloakMeter(PackRatios[GetPowerupSize()] * 100.0, false);

		bPackUsed = (iPrimaryAmmoGiven || iSecondaryAmmoGiven || iMetalGiven); // || iCloakGiven

		if (bPackUsed)
		{
			IGameEvent *pEvent = gameeventmanager->CreateEvent("item_pickup");
			if (pEvent)
			{
				pEvent->SetInt("userid", pOFPlayer->GetUserID());
				pEvent->SetString("item", GetAmmoPackName());
				gameeventmanager->FireEvent(pEvent);
			}

			CSingleUserRecipientFilter user(pPlayer);
			user.MakeReliable();
			EmitSound(user, entindex(), "AmmoPack.Touch");
		}
	}

	return bPackUsed;
}