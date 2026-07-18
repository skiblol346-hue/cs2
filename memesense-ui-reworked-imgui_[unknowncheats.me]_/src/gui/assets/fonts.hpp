#pragma once

#include "imgui.h"

struct ImFont;
struct ImGuiIO;

namespace fonts
{
	inline constexpr float kUiSize = 12.f;
	inline constexpr float kTabIconSize = 47.f;

	void Init(ImGuiIO& io) noexcept;

	ImFont* Verdana() noexcept;
	ImFont* VerdanaBold() noexcept;
	ImFont* TabIcons() noexcept;
	bool TabIconsLoaded() noexcept;

	ImVec2 CalcUiTextSize(const char* text) noexcept;
	ImVec2 CalcTabIconSize(const char* text) noexcept;
}
