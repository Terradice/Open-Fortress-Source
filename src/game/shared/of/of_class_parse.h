// ========= Copyright Open Fortress Developers, CC-BY-NC-SA ============
// Purpose: Implementation of ClassData
// Author(s): KaydemonLP
//

#pragma once

struct  OFPlayerClassData_t
{
	char			m_szName[128];
	char			m_szModel[128];
	char			m_szModelHWM[128];
	char			m_szViewmodel[128];
	char			m_szNameLocalized[128];
	float			m_flMaxSpeed;
	int				m_iMaxHealth;
	int				m_iMaxArmor;
	int				m_iWeaponIDs[OF_PLAYER_WEAPON_COUNT];

	// RE NOTE: Grenades only have 2 confirmed array size
	// The latter 4 bytes are only assumed to be part of it
	// They might be something entirely different
	int				m_iGrenades[OF_PLAYER_GRENADE_COUNT];
	int				m_iMaxAmmo[OF_AMMO_COUNT];
	int				m_iBuildable[OF_PLAYER_BUILDABLE_COUNT];
	bool			m_bDontDoAirwalk;
	bool			m_bDontDoNewJump;
	bool			m_bIsParsed;
//	undefined		undefined
	float			m_flCameraOffsetForward;
	float			m_flCameraOffsetRight;
	float			m_flCameraOffsetUp;
	char			m_szDeathSounds[12][128];
	
	void	AddAdditionalPlayerDeathSounds( void );
	char	*GetDeathSound( int iSoundIndex );
	char	*GetModelName( void );
	virtual void	ParseData( KeyValues *kvData );
};

// RE: This class is kind of speculative
// though it seems it's very basic middleman class
// which saves the class data
// so hopefully minor descrepancies won't break anything - Kay
class COFPlayerClassDataMgr : public CAutoGameSystem
{
public:
	virtual bool Init( void );

	OFPlayerClassData_t *Get( unsigned int iClassIndeex );
	void	AddAdditionalPlayerDeathSounds( void );
	
private:

	OFPlayerClassData_t m_ClassDatabase[OF_CLASS_COUNT];
};

extern OFPlayerClassData_t* GetPlayerClassData( uint32 iClassIndex );

// Need these to access in tf_player.cpp
#ifdef CLIENT_DLL
EXTERN_RECV_TABLE( DT_OFPlayerClassShared );
#else
EXTERN_SEND_TABLE( DT_OFPlayerClassShared );
#endif

class COFPlayerClassShared
{
public:

	COFPlayerClassShared();

	DECLARE_EMBEDDED_NETWORKVAR()
	DECLARE_CLASS_NOBASE( COFPlayerClassShared );

	bool	Init( int iClassIndex );
	void	Reset( void );
	
	bool	CanBuildObject( int iObjectIndex );

// Assuming this is only needed on the server
#ifdef GAME_DLL
	void	SetCustomModel( const char *szModelName, bool bCustomModelClassAnim );
#endif
	bool	CustomModelHasChanged( void );

	const char	*GetModelName( void );
	const char	*GetHandModelName( bool bUseGunslinger );

	OFPlayerClassData_t *GetClassData() { return GetPlayerClassData(m_iClass); }

protected:

	CNetworkVar( int, m_iClass );

#ifdef CLIENT_DLL
	char	m_szClassIcon[MAX_PATH];
	char	m_szCustomModel[MAX_PATH];
#else
	CNetworkVar( string_t, m_szClassIcon );
	CNetworkVar( string_t, m_szCustomModel );
#endif

	CNetworkVar( Vector, m_vecCustomModelOffset );
	CNetworkVar( QAngle, m_angCustomModelRotation );
	CNetworkVar( int, m_iCustomModelParity );
	// Old Model Parity isnt networked so the client can update as well
	int m_iOldCustomModelParity;
	CNetworkVar( bool, m_bCustomModelRotates );
	CNetworkVar( bool, m_bCustomRotationSet );
	CNetworkVar( bool, m_bCustomModelVisibleToSelf );
	CNetworkVar( bool, m_bCustomModelClassAnim );

	friend class COFPlayer;
	friend class C_OFPlayer;
};
