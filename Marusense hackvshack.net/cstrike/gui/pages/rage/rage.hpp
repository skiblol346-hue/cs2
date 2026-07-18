#pragma once

constexpr int kRageWeaponCount = 8;

// 0 = Global, 1 = Auto Sniper, 2 = SSG 08, 3 = AWP,
// 4 = R8 Revolver, 5 = Desert Eagle, 6 = Pistol, 7 = Rifle
inline int GetRageWeaponCategory(int nWeaponDefIndex)
{
	switch (nWeaponDefIndex)
	{
	case 38: case 11:
		return 1; // Auto Sniper (SCAR-20, G3SG1)
	case 40:
		return 2; // SSG 08
	case 9:
		return 3; // AWP
	case 64:
		return 4; // R8 Revolver
	case 1:
		return 5; // Desert Eagle
	case 4: case 61: case 32: case 2: case 36: case 3: case 30: case 63:
		return 6; // Pistol (Glock, USP-S, P2000, Dualies, P250, Five-Seven, Tec-9, CZ75)
	case 7: case 16: case 60: case 8: case 39: case 10: case 13:
	case 17: case 19: case 23: case 24: case 26: case 33: case 34:
	case 14: case 28: case 25: case 27: case 29: case 35:
		return 7; // Rifle (+ SMGs, LMGs, Shotguns)
	default:
		return 0; // Global
	}
}

namespace pages
{
	void RenderRage() noexcept;
}

namespace vars
{
	extern int rageWeapon;

	// --- per-weapon settings (indexed by weapon category) ---
	extern bool rageEnabled[kRageWeaponCount];
	extern int  rageTargetHitboxes[kRageWeaponCount];
	extern bool rageMultipoint[kRageWeaponCount];
	extern int  rageMultipointHitboxes[kRageWeaponCount];
	extern int  rageMultipointScale[kRageWeaponCount];
	extern int  rageHitchance[kRageWeaponCount];
	extern int  rageMindmg[kRageWeaponCount];
	extern bool rageAutoWall[kRageWeaponCount];
	extern bool rageHitchanceOverride[kRageWeaponCount];
	extern bool rageMindmgOverride[kRageWeaponCount];
	extern int  rageMindmgOverrideVal[kRageWeaponCount];
	extern bool rageForceLethal[kRageWeaponCount];
	extern bool rageForceShoot[kRageWeaponCount];
	extern int  rageForceShootHitchance[kRageWeaponCount];
	extern bool ragePreferBodyAim[kRageWeaponCount];
	extern bool rageForceBaim[kRageWeaponCount];
	extern bool rageDelayShot[kRageWeaponCount];
	extern int  rageDelayShotTicks[kRageWeaponCount];
	extern bool rageQuickStop[kRageWeaponCount];
	extern int  rageQuickStopMode[kRageWeaponCount];
	extern bool rageAutoscope[kRageWeaponCount];
	extern int  rageMaxFov[kRageWeaponCount];
	extern int  rageAimSpeed[kRageWeaponCount];
	extern int  rageMinVisible[kRageWeaponCount];

	// --- keybinds (global) ---
	extern int kbRageEnabled;
	extern int kbRageMultipoint;
	extern int kbRageHitchanceOverride;
	extern int kbRageMindmgOverride;
	extern int kbRageForceLethal;
	extern int kbRageForceShoot;
	extern int kbRageForceBaim;
	extern int kbRageDelayShot;
	extern int kbRageQuickStop;

	// --- global settings (not per-weapon) ---
	extern int  rageAccuracyBoost;
	extern bool rageRefineShot;
	extern bool rageAaCorrection;
	extern bool rageAutofire;
	extern bool rageSilentAim;
	extern int  kbRageSilentAim;
	extern bool rageRemoveRecoil;
	extern bool rageRemoveSpread;
	extern bool rageForceRemoveSpread;
	extern int  kbRageForceRemoveSpread;
	extern bool rageDoubletap;
	extern int  kbRageDoubletap;
	extern bool rageQuickPeekAssist;
	extern int  kbRageQuickPeekAssist;
	extern int  rageQuickPeekMode;
	extern int  rageQuickPeekDistance;
	extern bool rageDuckPeekAssist;
	extern int  kbRageDuckPeekAssist;
	extern bool rageLogMissed;
	extern bool rageLimitAimStep;

	// --- anti-aim ---
	extern bool aaEnabled;
	extern int  kbAaEnabled;
	extern int  aaPitch;
	extern int  aaYawBase;
	extern int  aaYawOffset;
	extern bool aaJitter;
	extern int  kbAaJitter;
	extern bool aaSpin;
	extern int  kbAaSpin;
	extern bool aaFreestanding;
	extern int  kbAaFreestanding;
	extern int  aaStaticYaw;
	extern bool aaMouseOverride;
	extern int  kbAaMouseOverride;
	extern int  aaMouseView;
	extern int  aaMouseAmount;
	extern int  aaManualYaw;
	extern bool aaManualForward;
	extern int  kbAaManualForward;
	extern int  aaDisablers;

	// --- colors ---
	extern float rageQpColor[4];
	extern float rageLogColor[4];
	extern float rageMo1Color[4];
	extern float rageMo2Color[4];
}

// returns the weapon config slot to use: specific weapon if enabled, otherwise Global
inline int GetActiveRageConfig(int nWeaponDefIndex)
{
	int specific = GetRageWeaponCategory(nWeaponDefIndex);
	if (specific != 0 && vars::rageEnabled[specific])
		return specific;
	return 0;
}
