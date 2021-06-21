// ========= Copyright Open Fortress Developers, CC-BY-NC-SA ============
// Purpose: Implementation of CHealthKit
// Author(s): Cherry!
//

#pragma once

#include "of_powerup.h"

DECLARE_AUTO_LIST(IHealthKitAutoList);

class CHealthKit : public COFPowerup, public IHealthKitAutoList
{
public:
	DECLARE_CLASS(CHealthKit, COFPowerup);

	virtual void Spawn();
	virtual void Precache();
	virtual bool MyTouch(CBasePlayer *pPlayer);
	virtual EOFPowerupSize GetPowerupSize() { return OF_POWERUP_FULL; }
	virtual const char *GetDefaultPowerupModel() { return "models/items/medkit_large.mdl"; }
	virtual const char *GetHealthKitName() { return "medkit_large"; }
};

class CHealthKitMedium : public CHealthKit
{
public:
	DECLARE_CLASS(CHealthKitMedium, CHealthKit);

	virtual EOFPowerupSize GetPowerupSize() { return OF_POWERUP_MEDIUM; }
	virtual const char *GetDefaultPowerupModel() { return "models/items/medkit_medium.mdl"; }
	virtual const char *GetHealthKitName() { return "medkit_medium"; }
};

class CHealthKitSmall : public CHealthKit
{
public:
	DECLARE_CLASS(CHealthKitSmall, CHealthKit);

	virtual EOFPowerupSize GetPowerupSize() { return OF_POWERUP_SMALL; }
	virtual const char *GetDefaultPowerupModel() { return "models/items/medkit_small.mdl"; }
	virtual const char *GetHealthKitName() { return "medkit_small"; }
};