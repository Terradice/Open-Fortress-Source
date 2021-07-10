// ========= Copyright Open Fortress Developers, CC-BY-NC-SA ============
// Purpose: Implementation of CTFTeamSpawn
// Author(s): MaartenS11, Cherry!
//

#include "cbase.h"

DECLARE_AUTO_LIST(IOFTeamSpawnAutoList);

class CTeamControlPoint;
class CTeamControlPointRound;

class COFTeamSpawn : public CBaseEntity, public IOFTeamSpawnAutoList
{
public:
	DECLARE_CLASS(COFTeamSpawn, CBaseEntity);
	DECLARE_DATADESC();

	COFTeamSpawn();

	void Activate();
	bool IsDisabled() { return m_bDisabled; }
	void SetDisable(bool bDisable) { m_bDisabled = bDisable; }
	void InputEnable(inputdata_t &inputdata);
	void InputDisable(inputdata_t &inputdata);
	void InputRoundSpawn(inputdata_t &inputdata);
	CHandle<CTeamControlPoint> GetControlPoint() { return m_hControlPoint; }
	CHandle<CTeamControlPointRound> GetBlueControlPoint() { return m_hControlPointBlue; }
	CHandle<CTeamControlPointRound> GetRedControlPoint() { return m_hControlPointRed; }
	bool GetSpawnMode() { return m_nSpawnMode; }
	//void Think(); // DEBUGGING

private:

	bool m_bDisabled;
	bool m_nSpawnMode;
	string_t m_iszControlPointName;
	string_t m_iszRoundBlueSpawn;
	string_t m_iszRoundRedSpawn;

	CHandle<CTeamControlPoint> m_hControlPoint;
	CHandle<CTeamControlPointRound> m_hControlPointBlue;
	CHandle<CTeamControlPointRound> m_hControlPointRed;
};