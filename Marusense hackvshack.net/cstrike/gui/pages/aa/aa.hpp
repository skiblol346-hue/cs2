#pragma once

namespace pages
{
	void RenderAA() noexcept;
}

namespace vars
{
	extern bool aaEnabled;
	extern int aaPitch;
	extern int aaYawBase;
	extern int aaYaw;
	extern int aaYawJitter;
	extern int aaYawJitterAmount;
	extern int aaBodyYaw;
	extern bool aaEdgeYaw;
	extern bool aaFakelagEnabled;
	extern int aaFakelagAmount;
	extern int aaFakelagVariance;
	extern int aaFakelagLimit;
	extern bool aaOtherSlowMotion;
	extern int aaOtherSlowMotionType;
	extern int aaOtherLegMovement;
	extern bool aaOnshotaa;
}
