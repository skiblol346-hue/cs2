#include "keybind.hpp"
#include "../styling.hpp"
#include "../shell/shell.hpp"
#include "gui/assets/fonts.hpp"
#include "imgui.h"
#include "imgui_internal.h"

#include <cstdio>
#include <string>
#include <utility>
#include <vector>

static std::string s_capturingId;

struct KeybindReg
{
    int*  key;
    bool* value;
};
static std::vector<KeybindReg> s_registry;

static const char* KeyToLabel(int key) noexcept
{
    switch (key)
    {
    case ImGuiKey_LeftShift:   return "SHF";
    case ImGuiKey_RightShift:  return "SHF";
    case ImGuiKey_LeftAlt:     return "ALT";
    case ImGuiKey_RightAlt:    return "ALT";
    case ImGuiKey_LeftCtrl:    return "CTR";
    case ImGuiKey_RightCtrl:   return "CTR";
    case ImGuiKey_MouseLeft:   return "M1";
    case ImGuiKey_MouseRight:  return "M2";
    case ImGuiKey_MouseMiddle: return "M3";
    case ImGuiKey_MouseX1:     return "M4";
    case ImGuiKey_MouseX2:     return "M5";
    case ImGuiKey_Space:       return "SPC";
    case ImGuiKey_Delete:      return "DEL";
    case ImGuiKey_Tab:         return "TAB";
    case ImGuiKey_CapsLock:    return "CAP";
    default: break;
    }
    if (key >= ImGuiKey_A && key <= ImGuiKey_Z)
    {
        static char buf[2];
        buf[0] = static_cast<char>('A' + (key - ImGuiKey_A));
        buf[1] = '\0';
        return buf;
    }
    if (key >= ImGuiKey_0 && key <= ImGuiKey_9)
    {
        static char buf[2];
        buf[0] = static_cast<char>('0' + (key - ImGuiKey_0));
        buf[1] = '\0';
        return buf;
    }
    if (key >= ImGuiKey_F1 && key <= ImGuiKey_F12)
    {
        static char buf[4];
        std::snprintf(buf, sizeof(buf), "F%d", 1 + (key - ImGuiKey_F1));
        return buf;
    }
    return "?";
}

static int ScanPressedKey() noexcept
{
    for (int k = ImGuiKey_A; k <= ImGuiKey_Z; ++k)
        if (ImGui::IsKeyPressed(static_cast<ImGuiKey>(k), false)) return k;
    for (int k = ImGuiKey_0; k <= ImGuiKey_9; ++k)
        if (ImGui::IsKeyPressed(static_cast<ImGuiKey>(k), false)) return k;
    for (int k = ImGuiKey_F1; k <= ImGuiKey_F12; ++k)
        if (ImGui::IsKeyPressed(static_cast<ImGuiKey>(k), false)) return k;

    if (ImGui::IsKeyPressed(ImGuiKey_LeftShift,  false)) return ImGuiKey_LeftShift;
    if (ImGui::IsKeyPressed(ImGuiKey_LeftAlt,    false)) return ImGuiKey_LeftAlt;
    if (ImGui::IsKeyPressed(ImGuiKey_LeftCtrl,   false)) return ImGuiKey_LeftCtrl;
    if (ImGui::IsKeyPressed(ImGuiKey_Tab,        false)) return ImGuiKey_Tab;
    if (ImGui::IsKeyPressed(ImGuiKey_CapsLock,   false)) return ImGuiKey_CapsLock;

    if (ImGui::IsMouseClicked(ImGuiMouseButton_Middle)) return ImGuiKey_MouseMiddle;
    if (ImGui::IsMouseClicked(3))                       return ImGuiKey_MouseX1;
    if (ImGui::IsMouseClicked(4))                       return ImGuiKey_MouseX2;

    return 0;
}

namespace ksd
{

void DrawKeybindButton(const char* id, int* key, bool* value, float rowY) noexcept
{
    s_registry.push_back({ key, value });

    const bool capturing = !s_capturingId.empty() && s_capturingId == id;

    if (capturing)
    {
        if (ImGui::IsKeyPressed(ImGuiKey_Escape, false))
        {
            s_capturingId.clear();
        }
        else if (ImGui::IsKeyPressed(ImGuiKey_Delete, false))
        {
            *key = 0;
            s_capturingId.clear();
        }
        else
        {
            const int pressed = ScanPressedKey();
            if (pressed != 0)
            {
                *key = pressed;
                s_capturingId.clear();
            }
        }
    }

    constexpr float btnW     = 26.f;
    constexpr float btnH     = 11.f;
    constexpr float fontSize = 9.f;
    const float     btnX = ksd::PanelRightX() - btnW;
    const float     btnY = rowY + 1.f;
    const ImRect    btnBb(ImVec2(btnX, btnY), ImVec2(btnX + btnW, btnY + btnH));

    const bool hovered = btnBb.Contains(ImGui::GetMousePos()) && ksd::PopUpsAreClosed();
    const bool clicked = hovered && ImGui::IsMouseReleased(ImGuiMouseButton_Left);

    if (clicked)
    {
        if (!capturing)
            s_capturingId = id;
        else
            s_capturingId.clear();
    }

    ImDrawList* dl = ImGui::GetWindowDrawList();

    char label[16];
    if (capturing)
        std::snprintf(label, sizeof(label), "[...]");
    else if (*key == 0)
        std::snprintf(label, sizeof(label), "[-]");
    else
        std::snprintf(label, sizeof(label), "[%s]", KeyToLabel(*key));

    const ImVec2 tsz = fonts::Verdana()->CalcTextSizeA(fontSize, FLT_MAX, 0.f, label);
    dl->AddText(fonts::Verdana(), fontSize,
        btnBb.Min + ImVec2((btnW - tsz.x) * 0.5f, (btnH - tsz.y) * 0.5f),
        capturing ? style::rgba(220, 220, 220) : style::rgba(160, 160, 160),
        label);
}

void ProcessKeybinds() noexcept
{
    for (const KeybindReg& e : s_registry)
    {
        if (*e.key != 0 && ImGui::IsKeyPressed(static_cast<ImGuiKey>(*e.key), false))
            *e.value = !*e.value;
    }
    s_registry.clear();
}

}
