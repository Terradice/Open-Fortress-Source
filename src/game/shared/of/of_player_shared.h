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
	class C_OFPlayer;
#else
	class COFPlayer;
#endif

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

private:

	OuterClass *m_pOuter;

	float field_0x324;

};

#endif // !OF_PLAYER_SHARED_H