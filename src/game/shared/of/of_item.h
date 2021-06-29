// ========= Copyright Open Fortress Developers, CC-BY-NC-SA ============
// Purpose: Implementation of CTFItem
// Author(s): Cherry!
//

#pragma once

#ifdef CLIENT_DLL
	#include "c_props.h"
#else
	#include "props.h"
#endif

#ifdef CLIENT_DLL
	#define COFPlayer C_OFPlayer
	#define COFItem C_OFItem
#endif

class COFPlayer;

class COFItem : public CDynamicProp
{
public:
	DECLARE_CLASS(COFItem, CDynamicProp)
	DECLARE_NETWORKCLASS();

	virtual int GetItemID() const;

	virtual void Pickup(COFPlayer *pPlayer, bool bNoDraw);
	virtual void Drop(COFPlayer *pPlayer, bool bNoDraw, bool param_3 = false, bool param_4 = true);

	#ifdef CLIENT_DLL

	virtual bool ShouldDraw();
	virtual ShadowType_t ShadowCastType();
	virtual bool ShouldHideGlowEffect();

	#endif

};