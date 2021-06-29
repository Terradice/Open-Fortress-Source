// ========= Copyright Open Fortress Developers, CC-BY-NC-SA ============
// Purpose: Implementation of C_TFPlayer
// Author(s): Fenteale
//

#pragma once

#include "c_baseplayer.h"
#include "of_playeranimstate.h"
#include "of_player_shared.h"
#include "of_class_parse.h"
#include "of_item.h"

class C_OFWeaponBase;

class C_OFPlayer : public C_BasePlayer 
{
public:
	DECLARE_CLASS( C_OFPlayer, C_BasePlayer );
	DECLARE_CLIENTCLASS();
	DECLARE_PREDICTABLE();
	DECLARE_INTERPOLATION();

	C_OFPlayer();
	void DoAnimationEvent( PlayerAnimEvent_t event, int nData = 0 );
	C_OFWeaponBase *GetActiveOFWeapon() const;
	C_OFWeaponBase *Weapon_OwnsThisID(int param_1) const;

	bool ShouldAutoReload(){ return false; };

	void SetOffHandWeapon(C_OFWeaponBase *pWeapon);
	void HolsterOffHandWeapon();

	static C_OFPlayer* GetLocalOFPlayer();
	const QAngle &C_OFPlayer::EyeAngles();

	virtual void FireBullet(
		Vector vecSrc,	// shooting postion
		const QAngle &shootAngles,  //shooting angle
		float vecSpread, // spread vector
		int iDamage, // base damage
		int iBulletType, // ammo type
		CBaseEntity *pevAttacker, // shooter
		bool bDoEffects,	// create impact effect ?
		float x,	// spread x factor
		float y	// spread y factor
		);

	void OnPreDataChanged(DataUpdateType_t updateType);
	void OnDataChanged(DataUpdateType_t updateType);

	void ValidateModelIndex();
	void OnPlayerClassChange();
	void UpdateClientSideAnimation();
	const QAngle &GetRenderAngles();

	COFPlayerAnimState *m_PlayerAnimState;

	COFPlayerShared m_Shared;
	friend class COFPlayerShared;

	float GetCritMult() { return m_Shared.GetCritMult(); };
	void SetItem(COFItem *pItem);
	bool HasItem() const;
	COFItem *GetItem() const;
	void SetSpeedOF();
	Vector &GetClassEyeHeight();
	void RemoveDisguise();
	void SetAnimation(PLAYER_ANIM playerAnim);

public:
	CNetworkVarEmbedded( COFPlayerClassShared, m_Class );
private:
	CNetworkHandle(COFItem, m_hItem);
	CNetworkHandle(C_OFWeaponBase, m_hOffHandWeapon);
	int m_iPreDataChangeTeam;
	int m_iPreDataChangeClass;
	QAngle m_angEyeAngles;
	CInterpolatedVar< QAngle > m_iv_angEyeAngles;
};

inline C_OFPlayer *ToOFPlayer( C_BaseEntity *pEntity )
{
	if ( !pEntity )
		return nullptr;
	
	if( !pEntity->IsPlayer() )
		return nullptr;

	return static_cast< C_OFPlayer* >( pEntity );
}