#pragma once

#include "imgui.h"
#include "gui/gui.hpp"
#include <algorithm>
#include <cmath>

namespace style
{
	inline int menuAlpha = 255;

	inline ImU32 rgba(int r, int g, int b, int a = -1)
	{
		if (a < 0)
			a = menuAlpha;
		return IM_COL32(r, g, b, std::clamp(a, 0, 255));
	}

	inline ImU32 glass(int r, int g, int b, float opacity)
	{
		return IM_COL32(r, g, b, std::clamp(static_cast<int>(menuAlpha * opacity), 0, 255));
	}

	inline float glassWindow = 0.20f;
	inline float glassPanel  = 0.50f;
	inline float glassWidget = 0.22f;
	inline float glassPopup  = 0.50f;
	inline float glassHover  = 0.28f;

	inline int outlineAlpha = 60;

	inline ImU32 accentU32(int alphaOverride = -1)
	{
		const int a = alphaOverride >= 0 ? alphaOverride : menuAlpha;
		return IM_COL32(
			static_cast<int>(vars::colorAccent[0] * 255.f),
			static_cast<int>(vars::colorAccent[1] * 255.f),
			static_cast<int>(vars::colorAccent[2] * 255.f),
			std::min(static_cast<int>(vars::colorAccent[3] * 255.f), a));
	}

	inline int lerpChannel(int a, int b, float t)
	{
		return static_cast<int>(a + (b - a) * t);
	}

	inline ImU32 lerpColor(ImU32 c0, ImU32 c1, float t)
	{
		const int r0 = (c0 >> IM_COL32_R_SHIFT) & 0xFF;
		const int g0 = (c0 >> IM_COL32_G_SHIFT) & 0xFF;
		const int b0 = (c0 >> IM_COL32_B_SHIFT) & 0xFF;
		const int a0 = (c0 >> IM_COL32_A_SHIFT) & 0xFF;
		const int r1 = (c1 >> IM_COL32_R_SHIFT) & 0xFF;
		const int g1 = (c1 >> IM_COL32_G_SHIFT) & 0xFF;
		const int b1 = (c1 >> IM_COL32_B_SHIFT) & 0xFF;
		const int a1 = (c1 >> IM_COL32_A_SHIFT) & 0xFF;
		return IM_COL32(
			lerpChannel(r0, r1, t),
			lerpChannel(g0, g1, t),
			lerpChannel(b0, b1, t),
			lerpChannel(a0, a1, t));
	}

	inline void drawGradientH(ImDrawList* dl, ImVec2 min, ImVec2 max, ImU32 left, ImU32 right)
	{
		dl->AddRectFilledMultiColor(min, max, left, right, right, left);
	}

	inline void drawGradientV(ImDrawList* dl, ImVec2 min, ImVec2 max, ImU32 top, ImU32 bottom)
	{
		dl->AddRectFilledMultiColor(min, max, top, top, bottom, bottom);
	}

	namespace col
	{
		inline ImU32 outline()       { return rgba(12, 12, 12, std::min(outlineAlpha, menuAlpha)); }
		inline ImU32 outlineStrong() { return rgba(12, 12, 12, std::min(outlineAlpha + 70, menuAlpha)); }

		inline ImU32 bg()          { return glass(24, 24, 28, 0.16f); }
		inline ImU32 sidebarBg()   { return glass(18, 18, 22, 0.22f); }
		inline ImU32 separator()   { return outline(); }
		inline ImU32 outerBorder() { return outline(); }

		inline ImU32 accentA()     { return rgba(55,  177, 218); }
		inline ImU32 accentB()     { return rgba(204, 70,  205); }
		inline ImU32 accentC()     { return rgba(204, 227, 53);  }
		inline ImU32 accentLime()  { return rgba(150, 232, 0);   }

		inline ImU32 border()      { return outline(); }
		inline ImU32 widgetBorder(){ return outline(); }
		inline ImU32 comboPreview(){ return rgba(157, 157, 157); }
		inline ImU32 panel()       { return glass(14, 14, 18, 0.20f); }
		inline ImU32 panelDark()   { return glass(14, 14, 18, 0.20f); }
		inline ImU32 childBorder(bool resizing) { return resizing ? accentU32() : outline(); }

		inline ImU32 text()        { return rgba(205, 205, 205); }
		inline ImU32 textTitle()   { return rgba(205, 205, 205); }
		inline ImU32 textSpecial() { return rgba(182, 182, 101); }
		inline ImU32 textDim()     { return rgba(128, 128, 128); }
		inline ImU32 textShadow()  { return rgba(8,   8,   8);   }

		inline ImU32 gray40()      { return glass(78,  78,  86,  0.35f); }
		inline ImU32 gray65()      { return glass(150, 150, 160, 0.70f); }

		inline ImU32 tabIdle()     { return rgba(62,  62,  62);  }
		inline ImU32 tabHover()    { return rgba(118, 118, 118); }
		inline ImU32 tabActive()   { return rgba(210, 210, 210); }

		inline ImU32 chkIdleTop()  { return glass(77, 77, 77, glassWidget); }
		inline ImU32 chkIdleBot()  { return glass(47, 47, 47, glassWidget); }
		inline ImU32 chkHoverTop() { return glass(85, 85, 85, glassWidget); }
		inline ImU32 chkHoverBot() { return glass(55, 55, 55, glassWidget); }

		inline ImU32 sldTrackTop()      { return rgba(38, 38, 38); }
		inline ImU32 sldTrackBot()      { return rgba(50, 50, 50); }
		inline ImU32 sldTrackHoverTop() { return rgba(52, 52, 52); }
		inline ImU32 sldTrackHoverBot() { return rgba(64, 64, 64); }

		inline ImU32 titleCyan()    { return rgba(55,  177, 218); }
		inline ImU32 titleMagenta() { return rgba(204, 70,  205); }
		inline ImU32 titleLime()    { return rgba(204, 227, 53);  }
	}
}
