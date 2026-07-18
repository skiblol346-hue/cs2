#pragma once

namespace ksd
{
	bool Checkbox(const char* label, bool* v, bool special = false) noexcept;
	bool CheckboxKeybind(const char* label, bool* v, int* keybind, bool special = false) noexcept;
}
