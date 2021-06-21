// ========= Copyright Open Fortress Developers, CC-BY-NC-SA ============
// Purpose: Implementation of CAmmoPack
// Author(s): Cherry!
//

#pragma once

#include "of_powerup.h"

class CAmmoPack : public COFPowerup
{
public:
	DECLARE_CLASS(CAmmoPack, COFPowerup);

	virtual void Spawn();
	virtual void Precache();
	virtual bool MyTouch(CBasePlayer *pPlayer);
	virtual EOFPowerupSize GetPowerupSize() { return OF_POWERUP_FULL; }
	virtual const char *GetDefaultPowerupModel() { return "models/items/ammopack_large.mdl"; }
	virtual const char *GetAmmoPackName() { return "ammopack_large"; }
};

class CAmmoPackMedium : public CAmmoPack
{
public:
	DECLARE_CLASS(CAmmoPackMedium, CAmmoPack);

	virtual EOFPowerupSize GetPowerupSize() { return OF_POWERUP_MEDIUM; }
	virtual const char *GetDefaultPowerupModel() { return "models/items/ammopack_medium.mdl"; }
	virtual const char *GetAmmoPackName() { return "ammopack_medium"; }
};

class CAmmoPackSmall : public CAmmoPack
{
public:
	DECLARE_CLASS(CAmmoPackSmall, CAmmoPack);

	virtual EOFPowerupSize GetPowerupSize() { return OF_POWERUP_SMALL; }
	virtual const char *GetDefaultPowerupModel() { return "models/items/ammopack_small.mdl"; }
	virtual const char *GetAmmoPackName() { return "ammopack_small"; }
};