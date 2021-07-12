// ========= Copyright Open Fortress Developers, CC-BY-NC-SA ============
// Purpose: Implementation of the particle effects. (Explosions, bullets, etc)
// Author(s): Cherry!
//

#include "cbase.h"
#include "of_fx_shared.h"
#include "of_shareddefs.h"
#include "c_of_player.h"
#include "c_basetempentity.h"
#include "of_weapon_base_gun.h"
#include <cliententitylist.h>
#include "engine/IEngineSound.h"
#include "tier0/vprof.h"

#define OFFX_NULL (MAX_EDICTS - 1)

// made this a function to make my life easier
COFWeaponInfo *GetWeaponInfo(int iWeaponID)
{
	const char *pszWeaponAlias = WeaponIDToAlias(iWeaponID);
	if (!pszWeaponAlias) return NULL;

	WEAPON_FILE_INFO_HANDLE hWeaponInfo = LookupWeaponInfoSlot(pszWeaponAlias);
	if (hWeaponInfo == GetInvalidWeaponInfoHandle()) return NULL;

	COFWeaponInfo *pWeaponInfo = dynamic_cast<COFWeaponInfo*>(GetFileWeaponInfoFromHandle(hWeaponInfo));
	return pWeaponInfo;
}

//-----------------------------------------------------------------------------
// Purpose: Display's a blood sprite
//-----------------------------------------------------------------------------
class C_TEFireBullets : public C_BaseTempEntity
{
public:
	DECLARE_CLASS( C_TEFireBullets, C_BaseTempEntity );
	DECLARE_CLIENTCLASS();

	virtual void	PostDataUpdate( DataUpdateType_t updateType );

public:
	int		m_iPlayer;
	Vector	m_vecOrigin;
	QAngle	m_vecAngles;
	int		m_iWeaponID;
	int		m_iMode;
	int		m_iSeed;
	float	m_flSpread;
};


void C_TEFireBullets::PostDataUpdate( DataUpdateType_t updateType )
{
	// Create the effect.
	
	m_vecAngles.z = 0;
	
	C_OFPlayer *pPlayer = dynamic_cast<C_OFPlayer*>( UTIL_PlayerByIndex(m_iPlayer+1));
	if( !pPlayer )
		return;

	C_OFWeaponBaseGun *pWeapon = dynamic_cast<C_OFWeaponBaseGun*> (pPlayer->GetActiveOFWeapon());
	if( !pWeapon )
		return;

	FX_FireBullets(						
		pWeapon,
		m_iPlayer+1,
		m_vecOrigin,
		m_vecAngles,
		m_iWeaponID,
		m_iMode,
		m_iSeed,
		m_flSpread,
		pWeapon->GetProjectileDamage(),
		false);
}

IMPLEMENT_CLIENTCLASS_EVENT( C_TEFireBullets, DT_TEFireBullets, CTEFireBullets );

BEGIN_RECV_TABLE_NOBASE(C_TEFireBullets, DT_TEFireBullets)
	RecvPropVector( RECVINFO( m_vecOrigin ) ),
	RecvPropFloat( RECVINFO( m_vecAngles[0] ) ),
	RecvPropFloat( RECVINFO( m_vecAngles[1] ) ),
	RecvPropInt( RECVINFO( m_iWeaponID ) ),
	RecvPropInt( RECVINFO( m_iMode ) ), 
	RecvPropInt( RECVINFO( m_iSeed ) ),
	RecvPropInt( RECVINFO( m_iPlayer ) ),
	RecvPropFloat( RECVINFO( m_flSpread ) ),
END_RECV_TABLE()

//-----------------------------------------------------------------------------
// Purpose: Kaboom!
//-----------------------------------------------------------------------------
class C_TEOFExplosion : public C_BaseTempEntity
{
public:
	DECLARE_CLASS(C_TEOFExplosion, C_BaseTempEntity);
	DECLARE_CLIENTCLASS();

	virtual void	PostDataUpdate(DataUpdateType_t updateType);

public:

	Vector m_vecOrigin;
	Vector m_vecNormal;
	int m_iWeaponID;
	ClientEntityHandle_t m_hEntIndex;

};

void OFExplosionCallback(Vector vecOrigin, Vector vecNormal, int iWeaponID, ClientEntityHandle_t hEntity)
{
	COFWeaponInfo *pWeaponInfo = NULL;
	switch (iWeaponID)
	{
	case OF_WEAPON_GRENADE_DEMOMAN:
	case OF_WEAPON_GRENADE_PIPEBOMB:
		pWeaponInfo = GetWeaponInfo(OF_WEAPON_PIPEBOMBLAUNCHER);
		break;
	//case OF_WEAPON_FLAMETHROWER_ROCKET:
	//	pWeaponInfo = GetWeaponInfo(OF_WEAPON_FLAMETHROWER);
	//	break;
	default:
		pWeaponInfo = GetWeaponInfo(iWeaponID);
	}

	bool bIsPlayer = false;
	if (hEntity.Get())
	{
		C_BaseEntity *pEntity = C_BaseEntity::Instance(hEntity);
		if (pEntity) bIsPlayer = pEntity->IsPlayer();
	}

	bool bInWater = UTIL_PointContents(vecOrigin) & CONTENTS_WATER;
	bool bVar9 = false;
	QAngle angExplosion(0.0, 0.0, 0.0);
	if (0.05 > float(vecNormal.x) && 0.05 > float(vecNormal.y) && 0.05 > float(vecNormal.z))
	{
		angExplosion.Init();
		bVar9 = true;
	}
	else
	{
		VectorAngles(vecNormal, angExplosion);
		bVar9 = false;
	}

	// skipping halloween custom explosion sound and effect

	const char *pszExplosionEffect = "ExplosionCore_wall";
	const char *pszExplosionSound = "BaseExplosionEffect.Sound";

	if (pWeaponInfo)
	{
		if (bInWater)
		{
			if (V_strlen(pWeaponInfo->m_szExplosionEffectWater) > 0)
			{
				pszExplosionEffect = pWeaponInfo->m_szExplosionEffectWater;
			}
		}
		else
		{
			if (bIsPlayer || bVar9)
			{
				if (V_strlen(pWeaponInfo->m_szExplosionEffectPlayer) > 0)
				{
					pszExplosionEffect = pWeaponInfo->m_szExplosionEffectPlayer;
				}
			}
			else
			{
				if (V_strlen(pWeaponInfo->m_szExplosionEffect) > 0)
				{
					pszExplosionEffect = pWeaponInfo->m_szExplosionEffect;
				}
			}
		}

		// ECONNN

		if (V_strlen(pWeaponInfo->m_szExplosionSound) > 0)
		{
			pszExplosionSound = pWeaponInfo->m_szExplosionSound;
		}
	}

	CLocalPlayerFilter filter;
	C_BaseEntity::EmitSound(filter, SOUND_FROM_WORLD, pszExplosionSound, &vecOrigin);

	// pyro land vision filter here

	DispatchParticleEffect(pszExplosionEffect, vecOrigin, angExplosion);
}

void C_TEOFExplosion::PostDataUpdate(DataUpdateType_t updateType)
{
	VPROF("C_TEOFExplosion::PostDataUpdate");

	OFExplosionCallback(m_vecOrigin, m_vecNormal, m_iWeaponID, m_hEntIndex);
}

static void RecvProxy_ExplosionEntIndex(const CRecvProxyData *pData, void *pStruct, void *pOut)
{
	int iEntIndex = pData->m_Value.m_Int;
	((C_TEOFExplosion*)pStruct)->m_hEntIndex = (iEntIndex == -1 || iEntIndex == OFFX_NULL) ? INVALID_EHANDLE_INDEX : cl_entitylist->EntIndexToHandle(iEntIndex);
}

IMPLEMENT_CLIENTCLASS_EVENT(C_TEOFExplosion, DT_TEOFExplosion, CTEOFExplosion);

BEGIN_RECV_TABLE_NOBASE(C_TEOFExplosion, DT_TEOFExplosion)
	RecvPropVector(RECVINFO(m_vecOrigin)),
	RecvPropVector(RECVINFO(m_vecNormal)),
	RecvPropInt(RECVINFO(m_iWeaponID)),
	RecvPropInt("entindex", 0, SIZEOF_IGNORE, 0, RecvProxy_ExplosionEntIndex),
END_RECV_TABLE()