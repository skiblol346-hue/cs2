#include "keybind.hpp"
#include "../styling.hpp"
#include "../shell/shell.hpp"
#include "gui/assets/fonts.hpp"
#include "imgui.h"
#include "imgui_internal.h"

#include <cstdio>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

static std::string s_capturingId;

struct KeybindReg
{
    int*  key;
    bool* value;
    int*  modeOut;
};
static std::vector<KeybindReg> s_registry;

static std::unordered_map<const void*, int> s_modes;

static const void* s_popupId    = nullptr;
static ImRect      s_popupBtnBb = {};

static const void* WidgetId(const int* key, const bool* value) noexcept
{
    return value ? static_cast<const void*>(value)
                 : static_cast<const void*>(key);
}

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

    if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))   return ImGuiKey_MouseLeft;
    if (ImGui::IsMouseClicked(ImGuiMouseButton_Right))  return ImGuiKey_MouseRight;
    if (ImGui::IsMouseClicked(ImGuiMouseButton_Middle)) return ImGuiKey_MouseMiddle;
    if (ImGui::IsMouseClicked(3))                       return ImGuiKey_MouseX1;
    if (ImGui::IsMouseClicked(4))                       return ImGuiKey_MouseX2;

    return 0;
}

namespace ksd
{

void DrawKeybindButton(const char* id, int* key, bool* value, float rowY, int* modeOut) noexcept
{
    s_registry.push_back({ key, value, modeOut });

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
            int pressed = ScanPressedKey();
            // ignore left-click when hovering the keybind button itself to avoid misassignment
            constexpr float checkW = 26.f;
            const float checkX = ksd::PanelRightX() - checkW;
            const ImRect checkBb(ImVec2(checkX, rowY), ImVec2(checkX + checkW, rowY + 14.f));
            if (pressed == ImGuiKey_MouseLeft && checkBb.Contains(ImGui::GetMousePos()))
                pressed = 0;
            if (pressed != 0)
            {
                *key = pressed;
                s_capturingId.clear();
            }
        }
    }

    const void* wid  = WidgetId(key, value);
    int curMode = KBMode_Toggle;
    if (modeOut)
        curMode = *modeOut;
    else if (s_modes.count(wid))
        curMode = s_modes[wid];

    constexpr float btnW     = 26.f;
    constexpr float btnH     = 11.f;
    constexpr float fontSize = 9.f;
    const float     btnX = ksd::PanelRightX() - btnW;
    const float     btnY = rowY + 1.f;
    const ImRect    btnBb(ImVec2(btnX, btnY), ImVec2(btnX + btnW, btnY + btnH));

    const bool rawHovered = btnBb.Contains(ImGui::GetMousePos());
    const bool hovered    = rawHovered && ksd::PopUpsAreClosed();
    const bool clicked    = hovered && ImGui::IsMouseReleased(ImGuiMouseButton_Left);
    const bool rClicked   = rawHovered && ImGui::IsMouseReleased(ImGuiMouseButton_Right);

    if (clicked)
    {
        if (!capturing)
            s_capturingId = id;
        else
            s_capturingId.clear();
    }

    if (rClicked)
    {
        if (s_popupId == wid)
        {
            s_popupId = nullptr;
        }
        else
        {
            ksd::CloseAllPopups();
            s_popupId    = wid;
            s_popupBtnBb = btnBb;
            s_capturingId.clear();
        }
    }

    ImDrawList* dl = ImGui::GetWindowDrawList();

    char label[16];
    if (capturing)
        std::snprintf(label, sizeof(label), "[...]");
    else if (curMode == KBMode_AlwaysOn)
        std::snprintf(label, sizeof(label), "[ON]");
    else if (*key == 0)
        std::snprintf(label, sizeof(label), "[-]");
    else
        std::snprintf(label, sizeof(label), "[%s]", KeyToLabel(*key));

    const ImVec2 tsz = fonts::Verdana()->CalcTextSizeA(fontSize, FLT_MAX, 0.f, label);

    ImU32 labelCol;
    if (capturing)
        labelCol = style::rgba(220, 220, 220);
    else if (curMode == KBMode_AlwaysOn)
        labelCol = style::accentU32();
    else if (curMode == KBMode_Hold)
        labelCol = style::rgba(182, 182, 101);
    else
        labelCol = style::rgba(160, 160, 160);

    dl->AddText(fonts::Verdana(), fontSize,
        btnBb.Min + ImVec2((btnW - tsz.x) * 0.5f, (btnH - tsz.y) * 0.5f),
        labelCol, label);
}

void ProcessKeybinds() noexcept
{
    for (const KeybindReg& e : s_registry)
    {
        if (!e.key)
            continue;

        const void* wid  = WidgetId(e.key, e.value);
        int mode = KBMode_Toggle;
        if (e.modeOut)
            mode = *e.modeOut;
        else if (s_modes.count(wid))
            mode = s_modes[wid];

        if (e.value)
        {
            switch (mode)
            {
            case KBMode_Toggle:
                if (*e.key != 0 && ImGui::IsKeyPressed(static_cast<ImGuiKey>(*e.key), false))
                    *e.value = !*e.value;
                break;
            case KBMode_Hold:
                *e.value = *e.key != 0 && ImGui::IsKeyDown(static_cast<ImGuiKey>(*e.key));
                break;
            case KBMode_AlwaysOn:
                *e.value = true;
                break;
            }
        }
    }
    s_registry.clear();

    if (s_popupId)
    {
        static const char* kLabels[] = { "Toggle", "Hold", "Always on" };
        constexpr int   kCount = 3;
        constexpr float kPopW  = 70.f;
        constexpr float kRowH  = 18.f;
        constexpr float kPopH  = kRowH * kCount;

        const ImVec2 popPos(s_popupBtnBb.Min.x, s_popupBtnBb.Max.y + 2.f);
        const ImVec2 popMax = popPos + ImVec2(kPopW, kPopH);
        const ImRect popBb(popPos, popMax);

        ImDrawList* dl = ImGui::GetForegroundDrawList();

        ksd::BlurRegion(popPos, popMax);
        dl->AddRectFilled(popPos, popMax, style::glass(18, 18, 22, style::glassPopup));
        dl->AddRectFilled(popPos, popMax, style::col::panel());
        dl->AddRect(popPos, popMax, style::col::border());

        int curMode = s_modes.count(s_popupId) ? s_modes[s_popupId] : KBMode_Toggle;

        // find matching modeOut pointer
        int* matchedModeOut = nullptr;
        for (const KeybindReg& e : s_registry)
        {
            const void* wid = WidgetId(e.key, e.value);
            if (wid == s_popupId && e.modeOut)
            {
                matchedModeOut = e.modeOut;
                curMode = *e.modeOut;
                break;
            }
        }

        for (int i = 0; i < kCount; ++i)
        {
            const ImVec2 rowPos  = popPos + ImVec2(1.f, 1.f + i * kRowH);
            const ImVec2 rowSize(kPopW - 2.f, kRowH - 2.f);
            const ImRect rowBb(rowPos, rowPos + rowSize);
            const bool   rowHov = rowBb.Contains(ImGui::GetMousePos());

            if (rowHov)
                dl->AddRectFilled(rowPos, rowPos + rowSize,
                    style::glass(40, 40, 40, style::glassWidget));

            const ImVec2 textSz = fonts::CalcUiTextSize(kLabels[i]);
            dl->AddText(fonts::Verdana(), fonts::kUiSize,
                rowPos + ImVec2(6.f, (rowSize.y - textSz.y) * 0.5f),
                curMode == i ? style::accentU32() : style::col::text(),
                kLabels[i]);

            if (rowHov && ImGui::IsMouseReleased(ImGuiMouseButton_Left))
            {
                s_modes[s_popupId] = i;
                if (matchedModeOut)
                    *matchedModeOut = i;
                s_popupId = nullptr;
                break;
            }
        }

        if (s_popupId && ImGui::IsMouseReleased(ImGuiMouseButton_Left)
            && !popBb.Contains(ImGui::GetMousePos())
            && !s_popupBtnBb.Contains(ImGui::GetMousePos()))
        {
            s_popupId = nullptr;
        }
    }
}

bool AnyKeybindPopupOpen() noexcept
{
    return s_popupId != nullptr;
}

void CloseKeybindPopup() noexcept
{
    s_popupId = nullptr;
}

}
