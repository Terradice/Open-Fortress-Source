// ========= Copyright Open Fortress Developers, CC-BY-NC-SA ============
// Purpose: Implementation of CTFPlayerShared
// Author(s): Cherry!
//

#ifndef OF_PLAYER_SHARED_H

#define OF_PLAYER_SHARED_H

#ifdef _WIN32
#pragma once
#endif

#include "networkvar.h"

#ifdef CLIENT_DLL
	#define COFPlayer C_OFPlayer
#endif

class COFPlayer;

class COFPlayerShared
{
public:

#ifdef CLIENT_DLL
	friend class C_OFPlayer;
	typedef C_OFPlayer OuterClass;
#else
	friend class COFPlayer;
	typedef COFPlayer OuterClass;
#endif

	DECLARE_EMBEDDED_NETWORKVAR();
	DECLARE_CLASS_NOBASE( COFPlayerShared );

	COFPlayerShared();
	~COFPlayerShared();

	void Init(OuterClass *pOuter);

	virtual float GetCritMult();
	virtual bool CanAttack(int iFlags);
	//virtual bool InCond(ETFCond eCond) const;
	virtual bool IsAllowedToPickUpFlag() const;

private:

	OuterClass *m_pOuter;

	float field_0x324;
	bool m_bAllowedToPickUpFlag;
};

#endif // !OF_PLAYER_SHARED_H