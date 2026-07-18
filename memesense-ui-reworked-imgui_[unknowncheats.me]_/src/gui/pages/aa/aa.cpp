#include "aa.hpp"
#include "../../framework/shell/shell.hpp"

namespace vars
{
	int aaYaw = 0;
	int aaYawJitter = 0;
	int aaYawJitterAmount = 0;
	int aaBodyYaw = 0;
	bool aaEdgeYaw = false;
	bool aaFakelagEnabled = false;
	int aaFakelagAmount = 0;
	int aaFakelagVariance = 0;
	int aaFakelagLimit = 0;
	bool aaOtherSlowMotion = false;
	int aaOtherSlowMotionType = 0;
	int aaOtherLegMovement = 0;
	bool aaOnshotaa = true;
}

namespace pages
{
	void RenderAA() noexcept
	{
		if (ksd::BeginChild("Anti-aimbot angles"))
		{
			ksd::Checkbox("Enabled", &vars::aaEnabled);
			static const char* pitch[] = { "Off", "Default", "Up", "Down", "Minimal", "Random" };
			ksd::SingleSelect("Pitch", &vars::aaPitch, pitch, 6);
			static const char* yawBase[] = { "Local view", "At targets" };
			ksd::SingleSelect("Yaw base", &vars::aaYawBase, yawBase, 2);
			static const char* yaw[] = { "Off", "180", "Spin", "Static", "180 Z", "Crosshair" };
			ksd::SingleSelect("Yaw", &vars::aaYaw, yaw, 6);
			if (vars::aaYaw != 0)
			{
				static const char* jitter[] = { "Off", "Offset", "Center", "Random" };
				ksd::SingleSelect("Yaw jitter", &vars::aaYawJitter, jitter, 4);
				if (vars::aaYawJitter != 0)
					ksd::SliderInt("Yaw jitter amount", &vars::aaYawJitterAmount, -180, 180, "%d");
			}
			static const char* bodyYaw[] = { "Off", "Opposite", "Jitter", "Static" };
			ksd::SingleSelect("Body yaw", &vars::aaBodyYaw, bodyYaw, 4);
			ksd::Checkbox("Edge yaw", &vars::aaEdgeYaw);
		}
		ksd::EndChild();

		if (ksd::BeginChild("Fake lag", { 6, 0, 3, 5 }))
		{
			ksd::Checkbox("Enabled", &vars::aaFakelagEnabled);
			static const char* amount[] = { "Dynamic", "Maximum", "Fluctuate" };
			ksd::SingleSelect("Amount", &vars::aaFakelagAmount, amount, 3);
			ksd::SliderInt("Variance", &vars::aaFakelagVariance, 0, 100, "%d%%");
			ksd::SliderInt("Limit", &vars::aaFakelagLimit, 0, 15, "%d");
		}
		ksd::EndChild();

		if (ksd::BeginChild("Other#AA", { 6, 7, 3, 3 }))
		{
			ksd::Checkbox("Slow motion", &vars::aaOtherSlowMotion);
			static const char* slowType[] = { "Favor high speed", "Favor anti-aim" };
			ksd::SingleSelect("Slow motion type", &vars::aaOtherSlowMotionType, slowType, 2);
			static const char* legMove[] = { "Off", "Always slide", "Never slide" };
			ksd::SingleSelect("Leg movement", &vars::aaOtherLegMovement, legMove, 3);
			ksd::Checkbox("On shot anti-aim", &vars::aaOnshotaa);
			ksd::Label("Fake peek", true);
		}
		ksd::EndChild();
	}
}
