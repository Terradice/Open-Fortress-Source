// ========= Copyright Open Fortress Developers, CC-BY-NC-SA ============
// Purpose: Implementation of CTFHudWeaponAmmo
// Author(s): Cherry!
//

#pragma once

#include "hudelement.h"
#include "vgui_controls/EditablePanel.h"
#include "vgui_controls/ImagePanel.h"

class COFHudWeaponAmmo : public CHudElement, public vgui::EditablePanel
{
public:
	DECLARE_CLASS_SIMPLE(COFHudWeaponAmmo, vgui::EditablePanel);

	COFHudWeaponAmmo(const char *pElementName);
	void Reset();
	void ApplySchemeSettings(vgui::IScheme *pScheme);
	void UpdateAmmoLabels(bool bParam1, bool bParam2, bool bParam3);
	bool ShouldDraw();
	void ShowLowAmmoIndicator();
	virtual void OnThink();

private:

	CBaseCombatWeapon *field_0x1b0;

	int field_0x1b4;
	int field_0x1d8;

	COFLabel *m_pAmmoInClip;
	COFLabel *m_pAmmoInClipShadow;
	COFLabel *m_pAmmoInReserve;
	COFLabel *m_pAmmoInReserveShadow;
	COFLabel *m_pAmmoNoClip;
	COFLabel *m_pAmmoNoClipShadow;
	vgui::ImagePanel *m_pLowAmmoImage;

	float m_flNextThinkTime; // field_0x1ac

	int m_iLowAmmoX; // field_0x1bc
	int m_iLowAmmoY; // field_0x1c0
	int m_iLowAmmoWidth; // field_0x1c4
	int m_iLowAmmoHeight; // field_0x1c8
};