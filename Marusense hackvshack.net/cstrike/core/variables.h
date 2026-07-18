#pragma once

#include "config.h"

#pragma region variables_combo_entries
using VisualOverlayBox_t = int;

using VisualChamMaterial_t = int;
enum EVisualMaterials : int
{
	VISUAL_MATERIAL_PRIMARY_WHITE = 0,
	VISUAL_MATERIAL_ILLUMINATE,

	VISUAL_MATERIAL_LATEX,
	VISUAL_MATERIAL_GHOST,

	VISUAL_MATERIAL_MAX
};
enum ERagebotSpreadMode : int
{
	RAGE_SPREAD_SEED = 0,
	RAGE_SPREAD_HITCHANCE
};
using MiscDpiScale_t = int;


enum EHitboxes : int
{
	HITBOX_HEAD = (1 << 0),
	HITBOX_NECK = (1 << 1),
	HITBOX_CHEST = (1 << 2),
	HITBOX_PELVIS = (1 << 3)
};

enum EKeybindMode : int
{
	KBMODE_HOLD = 0,
	KBMODE_TOGGLE,
	KBMODE_ALWAYS_ON
};
#pragma endregion

#pragma region variables_multicombo_entries

#pragma endregion

struct Variables_t
{
#pragma region variables_Ragebot
	C_ADD_VARIABLE(bool, bRagebot, false);

	C_ADD_VARIABLE(float, flRageHitChance, 10.f);
	C_ADD_VARIABLE(float, flRagebotMinDamage, 30.f);
	C_ADD_VARIABLE(bool, bRagebotAutoWall, true);
	C_ADD_VARIABLE(bool, bRagebotAutoShoot, true);
	C_ADD_VARIABLE(int, nRagebotHitboxes, HITBOX_HEAD | HITBOX_NECK | HITBOX_CHEST);
#pragma endregion
#pragma region variables_visuals
	C_ADD_VARIABLE(bool, bVisualOverlay, false);

	C_ADD_VARIABLE(bool, bBox, true);

	C_ADD_VARIABLE(bool, bBoxOutline, true);

	C_ADD_VARIABLE(Color_t, colBox1, Color_t(255, 0, 0, 255));

	C_ADD_VARIABLE(bool, bSkeleton, true);

	C_ADD_VARIABLE(Color_t, colSkeleton1, Color_t(255, 255, 255, 255));
	C_ADD_VARIABLE(int, nRagebotSpreadMode, RAGE_SPREAD_HITCHANCE);

	C_ADD_VARIABLE(bool, bHealthBar, true);

	C_ADD_VARIABLE(bool, bHealthText, true);

	C_ADD_VARIABLE(Color_t, colHealth1, Color_t(0, 255, 0, 255));
	C_ADD_VARIABLE(Color_t, colHealth2, Color_t(255, 0, 0, 255));

	C_ADD_VARIABLE(Color_t, colHealthBarBg, Color_t(25, 25, 25, 255));
	C_ADD_VARIABLE(Color_t, colHealthBarBorder, Color_t(0, 0, 0, 255));

	C_ADD_VARIABLE(Color_t, colHealthText, Color_t(255, 255, 255, 255));
	C_ADD_VARIABLE(Color_t, colHealthTextOutline, Color_t(0, 0, 0, 255));
	C_ADD_VARIABLE(bool, bVisualChams, false);

	C_ADD_VARIABLE(int, nVisualChamMaterial, VISUAL_MATERIAL_PRIMARY_WHITE);
	C_ADD_VARIABLE(bool, bVisualChamsIgnoreZ, false);
	C_ADD_VARIABLE(Color_t, colVisualChams, Color_t(0, 255, 0));
	C_ADD_VARIABLE(bool, bVisualChamsWeapon, false);
	C_ADD_VARIABLE(bool, bVisualChamsArms, false);
	C_ADD_VARIABLE(int, nVisualChamMaterialWeapon, VISUAL_MATERIAL_PRIMARY_WHITE);
	C_ADD_VARIABLE(Color_t, colVisualChamsWeapon, Color_t(255, 255, 255, 255));

	C_ADD_VARIABLE(bool, bName, true);

	C_ADD_VARIABLE(Color_t, colPlayerName, Color_t(255, 255, 255, 255));
	C_ADD_VARIABLE(Color_t, colPlayerNameOutline, Color_t(0, 0, 0, 255));

	C_ADD_VARIABLE(bool, bWeapon, true);
	C_ADD_VARIABLE(bool, bSmokeRadius, true);
	C_ADD_VARIABLE(Color_t, colSmokeOutline, Color_t(255, 255, 255, 255));

	C_ADD_VARIABLE(Color_t, colSmokeFill, Color_t(255, 255, 255, 255));
	C_ADD_VARIABLE(Color_t, colInfernoOutline, Color_t(255, 255, 255, 255));
	C_ADD_VARIABLE(Color_t, colInfernoFill, Color_t(255, 255, 255, 255));

	C_ADD_VARIABLE(Color_t, colWeaponName, Color_t(255, 255, 255, 255));
	C_ADD_VARIABLE(Color_t, colWeaponNameOutline, Color_t(0, 0, 0, 255));

	C_ADD_VARIABLE(float, FFov, 90.0f);

	// ESP extras
	C_ADD_VARIABLE(bool, bSnaplines, false);
	C_ADD_VARIABLE(Color_t, colSnaplines, Color_t(255, 255, 255, 100));
	C_ADD_VARIABLE(bool, bHeadDot, false);
	C_ADD_VARIABLE(Color_t, colHeadDot, Color_t(255, 0, 0, 255));
	C_ADD_VARIABLE(bool, bAmmoBar, false);
	C_ADD_VARIABLE(bool, bArmorBar, false);
	C_ADD_VARIABLE(Color_t, colArmorBar, Color_t(0, 120, 255, 255));
	C_ADD_VARIABLE(bool, bFlags, false);
	C_ADD_VARIABLE(bool, bFlagArmor, true);
	C_ADD_VARIABLE(bool, bFlagScoped, true);
	C_ADD_VARIABLE(bool, bFlagDefusing, true);
	C_ADD_VARIABLE(bool, bFlagKit, true);
	C_ADD_VARIABLE(bool, bFlagFlashed, true);
	C_ADD_VARIABLE(bool, bDistance, false);

	// Extra visuals
	C_ADD_VARIABLE(bool, bGrenadePrediction, false);
	C_ADD_VARIABLE(Color_t, colGrenadePrediction, Color_t(0, 255, 0, 255));
	C_ADD_VARIABLE(bool, bBombTimer, false);
	C_ADD_VARIABLE(bool, bBombESP, false);
	C_ADD_VARIABLE(bool, bSpectatorList, false);
	C_ADD_VARIABLE(bool, bHitMarker, false);
	C_ADD_VARIABLE(bool, bDamageIndicator, false);
	C_ADD_VARIABLE(bool, bDroppedWeapons, false);
	C_ADD_VARIABLE(Color_t, colDroppedWeapons, Color_t(200, 200, 200, 255));

#pragma endregion

#pragma region variables_misc

	C_ADD_VARIABLE(bool, bInfernoRadius, false);
	C_ADD_VARIABLE(bool, bAntiFlash, false);
	C_ADD_VARIABLE(bool, bAntiSmoke, true);
	C_ADD_VARIABLE(bool, bAutoAccept, false);
	C_ADD_VARIABLE(bool, bEdgeBug, false);
	C_ADD_VARIABLE(unsigned int, nEdgeBugKey, VK_XBUTTON2);
	C_ADD_VARIABLE(int, nEdgeBugTicks, 16);
	C_ADD_VARIABLE(bool, bJumpBug, false);
	C_ADD_VARIABLE(unsigned int, nJumpBugKey, 0);
	C_ADD_VARIABLE(bool, bLastSecondDefuse, false);

	// Clantag
	C_ADD_VARIABLE(bool, bClantag, false);

	// Radar
	C_ADD_VARIABLE(bool, bRadarHack, false);

#pragma endregion

#pragma region variables_menu
	C_ADD_VARIABLE(unsigned int, nMenuKey, VK_INSERT);
	C_ADD_VARIABLE(unsigned int, nPanicKey, VK_END);


	/*
	 * color navigation:
	 * [definition N][purpose]
	 * 1. primitive:
	 * - primtv 0 (text)
	 * - primtv 1 (background)
	 * - primtv 2 (disabled)
	 * - primtv 3 (control bg)
	 * - primtv 4 (border)
	 * - primtv 5 (hover)
	 *
	 * 2. accents:
	 * - accent 0 (main)
	 * - accent 1 (dark)
	 * - accent 2 (darker)
	 */

	C_ADD_VARIABLE(float, flAnimationSpeed, 1.f);


#pragma endregion
#pragma region variables_SemiRagebot

	C_ADD_VARIABLE(bool, bSemiRagebot, false);
	C_ADD_VARIABLE(int, nHitboxes, HITBOX_HEAD);
	C_ADD_VARIABLE(bool, bDrawFov, false);
	C_ADD_VARIABLE(Color_t, colFov, Color_t(0, 255, 0));
	C_ADD_VARIABLE(bool, bBhop, false);


		C_ADD_VARIABLE(float, flAimRange, 10.0f);

	// Seed triggerbot
	C_ADD_VARIABLE(bool, bTriggerBot, false);
	C_ADD_VARIABLE(bool, bTriggerAutoWall, false);
	C_ADD_VARIABLE(bool, bTriggerAlways, false);

	C_ADD_VARIABLE(unsigned int, nTriggerKey, VK_XBUTTON1);
	C_ADD_VARIABLE(int, nTriggerKeyMode, KBMODE_HOLD);
	C_ADD_VARIABLE(int, nTriggerHitchance, 0);
	C_ADD_VARIABLE(int, nTriggerMinDamage, 0);
	C_ADD_VARIABLE(int, nReactionTime, 0);
	C_ADD_VARIABLE(int, nTriggerDelay, 1);

	// Legit aimbot
	C_ADD_VARIABLE(bool, bLegitAimbot, false);
	C_ADD_VARIABLE(unsigned int, nLegitAimbotKey, VK_XBUTTON2);
	C_ADD_VARIABLE(int, nLegitAimbotKeyMode, KBMODE_HOLD);
	C_ADD_VARIABLE(float, flLegitFov, 5.0f);
	C_ADD_VARIABLE(float, flLegitSmooth, 10.0f);
	C_ADD_VARIABLE(int, nLegitHitbox, 0);
	C_ADD_VARIABLE(bool, bLegitRCS, true);
	C_ADD_VARIABLE(float, flLegitRCSAmount, 80.0f);
	C_ADD_VARIABLE(int, nLegitShotDelay, 0);
	C_ADD_VARIABLE(int, nLegitKillDelay, 0);
	C_ADD_VARIABLE(bool, bLegitAutoWall, false);
	C_ADD_VARIABLE(bool, bLegitScopeOnly, false);
	C_ADD_VARIABLE(float, flLegitJitter, 0.0f);
	C_ADD_VARIABLE(bool, bLegitFlashCheck, true);
	C_ADD_VARIABLE(float, flLegitDeadzone, 0.3f);
	C_ADD_VARIABLE(bool, bLegitHumanized, true);

#pragma endregion
#pragma region variables_viewmodel
	C_ADD_VARIABLE(bool, viewmodell, false);
	C_ADD_VARIABLE(float, viewx, 0.0f);
	C_ADD_VARIABLE(float, viewy, 0.0f);
	C_ADD_VARIABLE(float, viewz, 0.0f);

	C_ADD_VARIABLE(float, viewfov, 68.0f);
#pragma endregion
};

inline Variables_t Vars = {};
