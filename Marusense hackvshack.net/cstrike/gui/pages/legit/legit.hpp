#pragma once

constexpr int kWeaponCount = 6;

// 0=Pistols, 1=SMGs, 2=Rifles, 3=Shotguns, 4=LMGs, 5=Snipers
// (tab icons usp_silencer/p90/ak47/mag7/negev/ssg08 are representative of each class)
inline int GetLegitWeaponCategory(int nWeaponDefIndex)
{
	switch (nWeaponDefIndex)
	{
	case 1: case 2: case 3: case 4: case 30: case 32: case 36: case 61: case 63: case 64:
		return 0; // Pistols (Deagle, Dualies, Five-Seven, Glock, Tec-9, P2000, P250, USP-S, CZ75, R8)
	case 17: case 19: case 23: case 24: case 26: case 33: case 34:
		return 1; // SMGs (MAC-10, P90, MP5-SD, UMP-45, PP-Bizon, MP7, MP9)
	case 7: case 8: case 10: case 13: case 16: case 39: case 60:
		return 2; // Rifles (AK-47, AUG, FAMAS, Galil AR, M4A4, SG 553, M4A1-S)
	case 25: case 27: case 29: case 35:
		return 3; // Shotguns (XM1014, MAG-7, Sawed-Off, Nova)
	case 14: case 28:
		return 4; // LMGs (M249, Negev)
	case 9: case 11: case 38: case 40:
		return 5; // Snipers (AWP, G3SG1, SCAR-20, SSG 08)
	default:
		return -1;
	}
}

namespace pages
{
	void RenderLegit() noexcept;
}

namespace vars
{
	extern int legitWeaponTab;

	extern bool  ltEnabled[kWeaponCount];
	extern int   ltHitboxes[kWeaponCount];
	extern float ltFov[kWeaponCount];
	extern bool  ltDrawFov[kWeaponCount];
	extern bool  ltAutoWall[kWeaponCount];

	extern bool  ltTrigger[kWeaponCount];
	extern bool  ltTriggerAutoWall[kWeaponCount];
	extern bool  ltTriggerAlways[kWeaponCount];
	extern int   ltTriggerKey[kWeaponCount];
	extern int   ltTriggerKeyMode[kWeaponCount];
	extern int   ltTriggerReaction[kWeaponCount];
	extern int   ltTriggerDelay[kWeaponCount];
}
