#pragma once

namespace pages
{
	void RenderRage() noexcept;
}

namespace vars
{
	extern int rageWeapon;
	extern int kbRageEnabled;
	extern bool rageEnabled;
	extern int rageTargetHitboxes;
	extern int kbRageMultipoint;
	extern bool rageMultipoint;
	extern int rageMultipointHitboxes;
	extern int rageMultipointScale;
	extern int rageHitchance;
	extern int rageMindmg;
	extern bool rageHitchanceOverride;
	extern int kbRageHitchanceOverride;
	extern bool rageMindmgOverride;
	extern int kbRageMindmgOverride;
	extern int rageMindmgOverrideVal;
	extern bool rageForceLethal;
	extern int kbRageForceLethal;
	extern bool rageForceShoot;
	extern int kbRageForceShoot;
	extern int rageForceShootHitchance;
	extern bool ragePreferBodyAim;
	extern bool rageForceBaim;
	extern int kbRageForceBaim;
	extern bool rageDelayShot;
	extern int kbRageDelayShot;
	extern int rageDelayShotTicks;
	extern bool rageQuickStop;
	extern int kbRageQuickStop;
	extern int rageQuickStopMode;
	extern bool rageAutoscope;
	extern int rageAccuracyBoost;
	extern bool rageRefineShot;
	extern bool rageAaCorrection;
	extern bool rageAutofire;
	extern bool rageAimThroughWalls;
	extern bool rageSilentAim;
	extern int kbRageSilentAim;
	extern bool rageRemoveRecoil;
	extern bool rageRemoveSpread;
	extern bool rageForceRemoveSpread;
	extern int kbRageForceRemoveSpread;
	extern bool rageDoubletap;
	extern int kbRageDoubletap;
	extern bool rageQuickPeekAssist;
	extern int kbRageQuickPeekAssist;
	extern int rageQuickPeekMode;
	extern int rageQuickPeekDistance;
	extern bool rageDuckPeekAssist;
	extern int kbRageDuckPeekAssist;
	extern bool rageLogMissed;
	extern int rageMaxFov;
	extern int rageAimSpeed;
	extern int rageMinVisible;
	extern bool rageLimitAimStep;
	extern bool aaEnabled;
	extern int kbAaEnabled;
	extern int aaPitch;
	extern int aaYawBase;
	extern int aaYawOffset;
	extern bool aaJitter;
	extern int kbAaJitter;
	extern bool aaSpin;
	extern int kbAaSpin;
	extern bool aaFreestanding;
	extern int kbAaFreestanding;
	extern int aaStaticYaw;
	extern bool aaMouseOverride;
	extern int kbAaMouseOverride;
	extern int aaMouseView;
	extern int aaMouseAmount;
	extern int aaManualYaw;
	extern bool aaManualForward;
	extern int kbAaManualForward;
	extern int aaDisablers;
	extern float rageQpColor[4];
	extern float rageLogColor[4];
	extern float rageMo1Color[4];
	extern float rageMo2Color[4];
}
