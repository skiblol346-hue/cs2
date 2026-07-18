#include "fonts.hpp"
#include "font_embedded.hpp"
#include "imgui.h"

#include <string>

namespace
{
	ImFont* g_Verdana = nullptr;
	ImFont* g_VerdanaBold = nullptr;
	ImFont* g_TabIcons = nullptr;

	ImFont* LoadTtfFromFile(ImFontAtlas* atlas, const char* path, float sizePx, ImFontConfig* cfg, const ImWchar* ranges)
	{
		if (!path || path[0] == '\0')
			return nullptr;
		return atlas->AddFontFromFileTTF(path, sizePx, cfg, ranges);
	}

	ImFont* LoadTtfFromMemory(ImFontAtlas* atlas, const unsigned char* data, int size, float sizePx, ImFontConfig* cfg, const ImWchar* ranges)
	{
		if (!data || size <= 0)
			return nullptr;
		return atlas->AddFontFromMemoryTTF(const_cast<unsigned char*>(data), size, sizePx, cfg, ranges);
	}
}

namespace fonts
{
	void Init(ImGuiIO& io) noexcept
	{
		static const ImWchar ranges[] = { 0x0020, 0x00FF, 0x2122, 0x2122, 0 };

		const std::string verdanaPath = "C:\\Windows\\Fonts\\verdana.ttf";
		const std::string verdanaBoldPath = "C:\\Windows\\Fonts\\verdanab.ttf";

		ImFontConfig uiCfg;
		uiCfg.OversampleH = uiCfg.OversampleV = 1;
		uiCfg.PixelSnapH = true;

		g_Verdana = LoadTtfFromFile(io.Fonts, verdanaPath.c_str(), kUiSize, &uiCfg, ranges);
		g_VerdanaBold = LoadTtfFromFile(io.Fonts, verdanaBoldPath.c_str(), kUiSize, &uiCfg, ranges);
		if (!g_VerdanaBold)
			g_VerdanaBold = g_Verdana;

		ImFontConfig iconCfg;
		iconCfg.OversampleH = iconCfg.OversampleV = 1;
		iconCfg.PixelSnapH = true;
		iconCfg.FontDataOwnedByAtlas = true;

		g_TabIcons = LoadTtfFromMemory(io.Fonts,
			font_embedded::tab_icons_data, font_embedded::tab_icons_size, kTabIconSize, &iconCfg, ranges);
		if (!g_TabIcons)
			g_TabIcons = g_Verdana;

		io.FontDefault = g_Verdana ? g_Verdana : io.Fonts->AddFontDefault();
	}

	ImFont* Verdana() noexcept { return g_Verdana; }
	ImFont* VerdanaBold() noexcept { return g_VerdanaBold ? g_VerdanaBold : g_Verdana; }
	ImFont* TabIcons() noexcept { return g_TabIcons ? g_TabIcons : g_Verdana; }

	bool TabIconsLoaded() noexcept { return g_TabIcons && g_TabIcons != g_Verdana; }

	ImVec2 CalcUiTextSize(const char* text) noexcept
	{
		return Verdana()->CalcTextSizeA(kUiSize, FLT_MAX, 0.f, text);
	}

	ImVec2 CalcTabIconSize(const char* text) noexcept
	{
		return TabIcons()->CalcTextSizeA(kTabIconSize, FLT_MAX, 0.f, text);
	}
}
