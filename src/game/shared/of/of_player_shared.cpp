// ========= Copyright Open Fortress Developers, CC-BY-NC-SA ============
// Purpose: Implementation of CTFPlayerShared
// Author(s): Cherry!
//

#include "cbase.h"
#include "of_weapon_base.h"
#include "of_shareddefs.h"
#include "of_player_shared.h"
#include "of_gamerules.h"

#ifdef CLIENT_DLL
	#include "c_of_player.h"
#else
	#include "of_player.h"
#endif


//OFSTATUS: INCOMPLETE
COFPlayerShared::COFPlayerShared()
{
	field_0x324 = 0.0;
}

//OFSTATUS: INCOMPLETE
COFPlayerShared::~COFPlayerShared()
{
}

//OFSTATUS: INCOMPLETE
float COFPlayerShared::GetCritMult()
{
	float flCritCalc = field_0x324 * 0.003921569;

	flCritCalc = clamp(flCritCalc, 0, 1.0);

	return (flCritCalc * 3.0) + 1.0;
}