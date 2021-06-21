// ========= Copyright Open Fortress Developers, CC-BY-NC-SA ============
// Purpose: Implementation of CTFPowerup, base for health kits and ammo kits
// Author(s): Cherry!
//

#pragma once

#include "items.h"

class COFPowerup : public CItem
{
public:
	DECLARE_CLASS(COFPowerup, CItem);
	DECLARE_DATADESC();

	COFPowerup();

	virtual void Spawn();
	virtual void Precache();
	virtual void SetDisabled(bool bDisable);
	virtual void Materialize();
	virtual void Materialize_Internal();
	virtual CBaseEntity *Respawn();
	virtual bool ValidTouch(CBasePlayer *pPlayer);
	virtual bool MyTouch(CBasePlayer *pPlayer);
	virtual void DropSingleInstance(Vector &pVelocity, CBaseCombatCharacter *pOwner, float param_3, float param_4);
	virtual bool ItemCanBeTouchedByPlayer(CBasePlayer *pPlayer);
	virtual float GetLifeTime() { return 30.0; } // sandvich life time
	virtual float GetRespawnDelay();
	virtual EOFPowerupSize GetPowerupSize() { return OF_POWERUP_FULL; }
	virtual const char *GetPowerupModel();
	virtual const char *GetDefaultPowerupModel() { return "error.mdl"; }

	void InputEnable(inputdata_t &inputdata) { m_bDisabled = false; }
	void InputDisable(inputdata_t &inputdata) { m_bDisabled = true; }
	void InputToggle(inputdata_t &inputdata) { m_bDisabled = !m_bDisabled; }

private:

	bool m_bDisabled;
	bool m_bRespawning; // field_0x4ed
	bool m_bAutoMaterialize;
	string_t m_iszModel;

	bool field_0x4ee;
	float m_fThrowWaitTime; // field_0x4f4
};