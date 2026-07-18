#pragma once

namespace ksd
{
    void DrawKeybindButton(const char* id, int* key, bool* value, float rowY) noexcept;
    void ProcessKeybinds() noexcept;
}
