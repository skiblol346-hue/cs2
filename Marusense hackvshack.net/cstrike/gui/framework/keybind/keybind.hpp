#pragma once

namespace ksd
{
    enum KeybindMode : int { KBMode_Toggle = 0, KBMode_Hold = 1, KBMode_AlwaysOn = 2 };

    void DrawKeybindButton(const char* id, int* key, bool* value, float rowY, int* modeOut = nullptr) noexcept;
    void ProcessKeybinds() noexcept;
    bool AnyKeybindPopupOpen() noexcept;
    void CloseKeybindPopup() noexcept;
}
