#pragma once

namespace pages
{
	void RenderVisuals() noexcept;
}

namespace vars
{
	extern bool visEnemyEnabled;
	extern int kbVisEnemy;
	extern bool visBbox;
	extern bool visHealthBar;
	extern int visHealthStyle;
	extern bool visName;
	extern bool visAvatar;
	extern bool visFlags;
	extern int visFlagItems;
	extern bool visSkeleton;
	extern bool visWeapon;
	extern int visWeaponStyle;
	extern bool visGrenadeInv;
	extern bool visAmmo;
	extern bool visNextShot;
	extern bool visDistance;
	extern bool visOffscreen;
	extern int visOffscreenSize;
	extern int visOffscreenAlpha;
	extern bool visAimbot;
	extern bool visTracers;
	extern float visTracersTime;
	extern bool visChamsPlayer;
	extern int visChamsMode;
	extern bool visChamsWall;
	extern bool visGlow;
	extern bool visShadow;
	extern int visShadowMode;
	extern bool visShadowGlow;
	extern bool visOnShot;
	extern bool visRagdolls;

	extern float visBboxColor[2][4];
	extern float visHealthColor[4];
	extern float visNameColor[4];
	extern float visAvatarColor[4];
	extern float visFlagsColor[2][4];
	extern float visSkeletonColor[4];
	extern float visGrenadeColor[4];
	extern float visAmmoColor[4];
	extern float visNextShotColor[4];
	extern float visDistanceColor[4];
	extern float visOffscreenColor[4];
	extern float visAimbotColor[4];
	extern float visTracersColor[4];

	extern float visPlayerColor[4];
	extern float visWallPlayerColor[4];
	extern float visGlowColor[4];
	extern float visShadowColor[4];
	extern float visShadowGlowColor[4];
	extern float visOnShotColor[4];
	extern float visRagdollsColor[4];
}
