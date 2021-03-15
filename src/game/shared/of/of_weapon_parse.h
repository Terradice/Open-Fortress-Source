// ========= Copyright Open Fortress Developers, CC-BY-NC-SA ============
// Purpose: 
// Author(s): Nopey
//
#pragma once


#include "weapon_parse.h"
#include "networkvar.h"

struct WeaponInfoStruct_t
{
	int m_iDamage;
	float m_flRange;
	int m_iBullets;
	float m_flSpread;
	float m_flPunchAngle;
	
	float m_flTimeFireDelay;
	float m_flTimeIdle;
	float m_flTimeIdleEmpty;
	
	float m_flTimeReloadStart;
	float m_flTimeReload;
	
	bool m_bDrawCrosshair;
	int m_iAmmoPerShot;
	
	bool m_bUseRapidFireCrits;
	
	int m_iProjectileType;
	
	float m_flProjectileSpeed;
	
	float m_flSmackDelay;
};

//--------------------------------------------------------------------------------------------------------
class COFWeaponInfo : public FileWeaponInfo_t
{
public:
	DECLARE_CLASS_GAMEROOT( COFWeaponInfo, FileWeaponInfo_t );
	
	COFWeaponInfo();
	
	virtual void Parse( KeyValues *pKeyValuesData, const char *szWeaponName );

	// Should probably add a Function here to get these instead of directly accessing this
	CUtlVector<WeaponInfoStruct_t> m_WeaponModeInfo;
	
	char m_szAnimExtension[16];		// string used to generate player animations with this weapon

	bool m_bDoInstantEjectBrass;
	char m_szBrassModel[128]; // OF TODO: Figure the size out later - Kay
	
	int m_iWeaponType;
	
	bool m_bGrenade;
	float m_flDamageRadius;
	float m_flPrimerTime;
	
	bool m_bPlayGrenTimer;
	bool m_bLowerMainWeapon;
	
	bool m_bVMHasTeamSkins;
	bool m_bWMHasTeamSkins;
	
	char m_szMuzzleFlashModel[128];
	float m_flMuzzleFlashModelDuration;
	
	char m_szMuzzleFlashParticle[128];
	char m_szTracerParticle[128];
	
	char m_szExplosionSound[128];
	char m_szExplosionEffect[128];
	char m_szExplosionEffectPlayer[128];
	char m_szExplosionEffectWater[128];
	 
	bool m_bDontDrop;
};
