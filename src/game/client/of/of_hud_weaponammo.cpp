// ========= Copyright Open Fortress Developers, CC-BY-NC-SA ============
// Purpose: Implementation of CTFHudWeaponAmmo
// Author(s): Cherry!
//

#include "cbase.h"
#include "vgui/of_controls.h"
#include "of_weapon_base.h"
#include "of_hud_weaponammo.h"
#include "of_clientmode.h"
#include <vgui_controls/AnimationController.h>
#include "ihudlcd.h"
#include "c_of_player.h"

DECLARE_HUDELEMENT(COFHudWeaponAmmo);

// OFSTATUS: COMPLETE
COFHudWeaponAmmo::COFHudWeaponAmmo(const char *pElementName) : CHudElement(pElementName), BaseClass(NULL, "HudWeaponAmmo")
{
	SetParent(g_pClientMode->GetViewport());
	SetHiddenBits(HIDEHUD_PLAYERDEAD); // HIDEHUD_HEALTH | HIDEHUD_PLAYERDEAD - once the health hud is added replace the bits with this

	hudlcd->SetGlobalStat("(ammo_primary)", "0");
	hudlcd->SetGlobalStat("(ammo_secondary)","0");
	hudlcd->SetGlobalStat("(weapon_print_name)", "");
	hudlcd->SetGlobalStat("(weapon_name)", "");

	m_pAmmoInClip = NULL; //field_0x1cc
	m_pAmmoInClipShadow = NULL; //field_0x1d0
	m_pAmmoInReserve = NULL; //field_0x1df
	m_pAmmoInReserveShadow = NULL; //field_0x1d8
	m_pAmmoNoClip = NULL; //field_0x1dc
	m_pAmmoNoClipShadow = NULL; //field_0x1e0
	m_pLowAmmoImage = NULL; //field_0x1e4
	m_flNextThinkTime = 0.0;
}

// OFSTATUS: COMPLETE
void COFHudWeaponAmmo::Reset()
{
	m_flNextThinkTime = gpGlobals->curtime + 0.05;
}

// OFSTATUS: COMPLETE
void COFHudWeaponAmmo::ApplySchemeSettings(vgui::IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	LoadControlSettings("resource/UI/HudAmmoWeapons.res");

	m_pAmmoInClip = dynamic_cast<COFLabel*>(FindChildByName("AmmoInClip"));
	m_pAmmoInClipShadow = dynamic_cast<COFLabel*>(FindChildByName("AmmoInClipShadow"));

	m_pAmmoInReserve = dynamic_cast<COFLabel*>(FindChildByName("AmmoInReserve"));
	m_pAmmoInReserveShadow = dynamic_cast<COFLabel*>(FindChildByName("AmmoInReserveShadow"));

	m_pAmmoNoClip = dynamic_cast<COFLabel*>(FindChildByName("AmmoNoClip"));
	m_pAmmoNoClipShadow = dynamic_cast<COFLabel*>(FindChildByName("AmmoNoClipShadow"));

	m_pLowAmmoImage = dynamic_cast<ImagePanel*>(FindChildByName("HudWeaponLowAmmoImage"));

	if (m_pLowAmmoImage)
	{
		m_pLowAmmoImage->GetBounds(m_iLowAmmoX, m_iLowAmmoY, m_iLowAmmoWidth, m_iLowAmmoHeight);
	}

	field_0x1b4 = -1;
	field_0x1d8 = -1;
	field_0x1b0 = NULL;

	m_flNextThinkTime = 0.0;

	UpdateAmmoLabels(false, false, false);
}

// OFSTATUS: COMPLETE
void COFHudWeaponAmmo::UpdateAmmoLabels(bool bAmmoInClip, bool bAmmoInReserve, bool bAmmoNoClip)
{
	if (m_pAmmoInClip && m_pAmmoInClipShadow)
	{
		m_pAmmoInClip->SetVisible(bAmmoInClip);
		m_pAmmoInClipShadow->SetVisible(bAmmoInClip);
	}

	if (m_pAmmoInReserve && m_pAmmoInReserveShadow)
	{
		m_pAmmoInReserve->SetVisible(bAmmoInReserve);
		m_pAmmoInReserveShadow->SetVisible(bAmmoInReserve);
	}

	if (m_pAmmoNoClip && m_pAmmoNoClipShadow)
	{
		m_pAmmoNoClip->SetVisible(bAmmoNoClip);
		m_pAmmoNoClipShadow->SetVisible(bAmmoNoClip);
	}
}

// OFSTATUS: INCOMPLETE
bool COFHudWeaponAmmo::ShouldDraw()
{
	COFPlayer *pPlayer = C_OFPlayer::GetLocalOFPlayer();
	if (!pPlayer) return false;

	COFWeaponBase *pWeapon = pPlayer->GetActiveOFWeapon();
	if (!pWeapon) return false;

	if (pWeapon->GetWeaponID() == OF_WEAPON_MEDIGUN) return false;

	if (!pWeapon->UsesPrimaryAmmo()) return false;

	//if (pWeapon->GetPrimaryAmmoType() == 3) return false;

	return CHudElement::ShouldDraw();
}

// OFSTATUS: COMPLETE
void COFHudWeaponAmmo::ShowLowAmmoIndicator()
{
	if (m_pLowAmmoImage && m_pLowAmmoImage->IsVisible())
	{
		m_pLowAmmoImage->SetBounds(m_iLowAmmoX, m_iLowAmmoY, m_iLowAmmoWidth, m_iLowAmmoHeight);
		m_pLowAmmoImage->SetVisible(true);
		m_pLowAmmoImage->SetFgColor(Color(255,0,0,255));

		g_pClientMode->GetViewportAnimationController()->StartAnimationSequence(this, "HudLowAmmoPulse");
	}
}

// OFSTATUS: INCOMPLETE
void COFHudWeaponAmmo::OnThink()
{
	COFPlayer *pPlayer = COFPlayer::GetLocalOFPlayer();
	CBaseCombatWeapon *pWeapon = GetActiveWeapon();

	if (gpGlobals->curtime <= m_flNextThinkTime) return;

	if (!pWeapon)
	{
		hudlcd->SetGlobalStat("(weapon_print_name)", " ");
	}
	else
	{
		hudlcd->SetGlobalStat("(weapon_print_name)", pWeapon->GetPrintName());
	}

	if (!pWeapon)
	{
		hudlcd->SetGlobalStat("(weapon_name)", " ");
	}
	else
	{
		hudlcd->SetGlobalStat("(weapon_name)", pWeapon->GetName());
	}

	if (pPlayer && pWeapon && pWeapon->UsesPrimaryAmmo())
	{
		int iAmmo1 = pWeapon->Clip1(); // ivar10
		int iAmmo2 = pPlayer->GetAmmoCount(pWeapon->GetPrimaryAmmoType()); // ivar11

		if (iAmmo1 < 0)
		{
			iAmmo1 = pPlayer->GetAmmoCount(pWeapon->GetPrimaryAmmoType());
			iAmmo2 = 0;
		}

		hudlcd->SetGlobalStat("(ammo_primary)", VarArgs("%d", iAmmo1));
		hudlcd->SetGlobalStat("(ammo_secondary)", VarArgs("%d", iAmmo2));

		if (field_0x1b4 != iAmmo1 || field_0x1d8 != iAmmo2 || field_0x1b0 != pWeapon)
		{
			field_0x1b4 = iAmmo1;
			field_0x1d8 = iAmmo2;
			field_0x1b0 = pWeapon;

			if (!pWeapon->UsesClipsForAmmo1())
			{
				UpdateAmmoLabels(false, false, true);
				SetDialogVariable("Ammo", field_0x1b4);
			}
			else
			{
				UpdateAmmoLabels(true, true, false);
				SetDialogVariable("Ammo", field_0x1b4);
				SetDialogVariable("AmmoInReserve", field_0x1d8);
			}

			// iVar16 = (**(code **)(*piVar7 + 0x650))(piVar7);
			// ...

		}
	}
	else
	{
		hudlcd->SetGlobalStat("(ammo_primary)", "n/a");
		hudlcd->SetGlobalStat("(ammo_secondary)", "n/a");

		UpdateAmmoLabels(false, false, false);

		if (m_pLowAmmoImage && m_pLowAmmoImage->IsVisible())
		{
			m_pLowAmmoImage->SetBounds(m_iLowAmmoX, m_iLowAmmoY, m_iLowAmmoWidth, m_iLowAmmoHeight);
			m_pLowAmmoImage->SetVisible(false);
			g_pClientMode->GetViewportAnimationController()->StartAnimationSequence(this, "HudLowAmmoPulseStop");
		}

		field_0x1b4 = -1;
		field_0x1d8 = -1;
	}

	m_flNextThinkTime = gpGlobals->curtime + 0.1;
}