#include "ui/KeyNames.hpp"

#include "platform/ClientCore.hpp"
std::string get_key_name(int vk) {
	if (vk == 0)
		return "None";

	switch (vk) {
	case VK_TAB: return "Tab";
	case VK_LEFT: return "Left Arrow";
	case VK_RIGHT: return "Right Arrow";
	case VK_UP: return "Up Arrow";
	case VK_DOWN: return "Down Arrow";
	case VK_PRIOR: return "Page Up";
	case VK_NEXT: return "Page Down";
	case VK_HOME: return "Home";
	case VK_INSERT: return "Insert";
	case VK_DELETE: return "Delete";
	case VK_BACK: return "Backspace";
	case VK_SPACE: return "Space";
	case VK_RETURN: return "Enter";
	case VK_ESCAPE: return "Escape";
	case VK_OEM_COMMA: return "Comma";
	case VK_OEM_PERIOD: return "Period";
	case VK_CAPITAL: return "Caps Lock";
	case VK_SCROLL: return "Scroll Lock";
	case VK_NUMLOCK: return "Num Lock";
	case VK_SNAPSHOT: return "Print Screen";
	case VK_PAUSE: return "Pause";
	case VK_LSHIFT: return "Left Shift";
	case VK_LCONTROL: return "Left Ctrl";
	case VK_LMENU: return "Left Alt";
	case VK_RSHIFT: return "Right Shift";
	case VK_RCONTROL: return "Right Ctrl";
	case VK_RMENU: return "Right Alt";
	case VK_LBUTTON: return "Mouse 1";
	case VK_RBUTTON: return "Mouse 2";
	case VK_MBUTTON: return "Mouse 3";
	case VK_XBUTTON1: return "Mouse 4";
	case VK_XBUTTON2: return "Mouse 5";
	}

	if (vk >= VK_F1 && vk <= VK_F12)
		return "F" + std::to_string(vk - VK_F1 + 1);

	if ((vk >= '0' && vk <= '9') || (vk >= 'A' && vk <= 'Z'))
		return std::string(1, static_cast<char>(vk));

	char name[64]{};
	if (GetKeyNameTextA(MapVirtualKeyA(vk, MAPVK_VK_TO_VSC) << 16, name, sizeof(name)))
		return name;

	return "Unknown (" + std::to_string(vk) + ")";
}
