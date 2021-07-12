// ========= Copyright Open Fortress Developers, CC-BY-NC-SA ============
// Purpose: Implementation of the particle effects. (Explosions, bullets, etc)
// Author(s): Cherry!
//

#pragma once

#define OFFX_NULL (MAX_EDICTS - 1)

void TE_FireBullets(int iPlayerIndex, const Vector &vOrigin, const QAngle &vAngles, int iWeaponID,	int	iMode, int iSeed, float flSpread);
void TE_OFExplosion(IRecipientFilter &filter, float flDelay, Vector vecOrigin, Vector vecNormal, int iWeaponID, int iIndex); // those last three params in the decomp are just for halloween related it seems