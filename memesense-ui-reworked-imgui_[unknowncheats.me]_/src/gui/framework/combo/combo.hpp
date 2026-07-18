#pragma once

#include "imgui.h"

namespace ksd
{
	bool SingleSelect(const char* label, int* current, const char* const* items, int count, float width = 0.f, bool borderless = false, const char* const* icons = nullptr) noexcept;
	bool MultiSelect(const char* label, int* flags, const char* const* items, int count) noexcept;
	void Label(const char* label, bool special = false) noexcept;
	void ColorButton(const char* id, float col[4], const ImVec2& pos, const ImVec2& size) noexcept;
	bool AnyComboOpen() noexcept;
	void CloseAllPopups() noexcept;
	void RenderPendingPopups() noexcept;
}
