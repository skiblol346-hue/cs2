#include "sliders.hpp"
#include "../styling.hpp"
#include "../shell/shell.hpp"
#include "gui/assets/fonts.hpp"
#include "imgui.h"
#include "imgui_internal.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <unordered_map>

static std::unordered_map<ImGuiID, bool> s_sliderActive;

static bool SliderBehavior(const char* id, ImRect trackBb, float* v, float v_min, float v_max)
{
    const ImGuiID hid     = ImHashStr(id);
    const bool    hovered = trackBb.Contains(ImGui::GetMousePos());
    const bool    lmb     = ImGui::IsMouseDown(ImGuiMouseButton_Left);

    if (hovered && lmb && !s_sliderActive[hid] && ksd::SelectedItem()[0] == '\0')
    {
        s_sliderActive[hid] = true;
        ksd::SetSelectedItem(id);
    }

    if (lmb && s_sliderActive[hid] && std::strcmp(ksd::SelectedItem(), id) == 0)
    {
        const float t = std::clamp(
            (ImGui::GetMousePos().x - trackBb.Min.x) / std::max(trackBb.GetWidth() - 1.f, 1.f),
            0.f, 1.f);
        *v = v_min + t * (v_max - v_min);
        return true;
    }

    if (!lmb && s_sliderActive[hid])
    {
        s_sliderActive[hid] = false;
        ksd::SetSelectedItem("");
    }

    return hovered;
}

static void DrawSliderImpl(const char* label, const void* idPtr, float* v, float v_min, float v_max,
                            const char* format, float scale, bool isInt, int remove)
{
    const ImVec2 cursor    = ksd::CursorPos();
    const float  w         = ksd::ContentWidth();
    const bool   hasLabel  = (label && label[0] != '\0');
    const ImVec2 labelSize = fonts::CalcUiTextSize(label);
    const float  labelH    = hasLabel ? labelSize.y : 0.f;

    const ImVec2 frameMin = cursor + ImVec2(20.f, -3.f);
    const ImRect sliderBb(
        ImVec2(frameMin.x,     frameMin.y + labelH + 4.f),
        ImVec2(frameMin.x + w, frameMin.y + labelH + 10.f));

    const ImRect leftArrow(sliderBb.Min - ImVec2(9.f, 0.f),  sliderBb.Min + ImVec2(-1.f, 6.f));
    const ImRect rightArrow(sliderBb.Max + ImVec2(1.f, -6.f), sliderBb.Max + ImVec2(9.f, 0.f));

    char formatted[100] = {};
    if (!format && isInt)
        std::snprintf(formatted, sizeof(formatted), "%d", static_cast<int>(*v) - remove);
    else if (!format)
        std::snprintf(formatted, sizeof(formatted), "%f", *v);
    else if (isInt)
        std::snprintf(formatted, sizeof(formatted), format, static_cast<int>(*v) - remove);
    else
        std::snprintf(formatted, sizeof(formatted), format, *v);

    ImU32      trackCol = style::glass(42, 42, 42, style::glassWidget);
    const bool canInteract = ksd::PopUpsAreClosed();

    char sid[32];
    std::snprintf(sid, sizeof(sid), "sld_%p", idPtr);

    bool sliderHovered = false;
    if (canInteract)
    {
        sliderHovered = SliderBehavior(sid, sliderBb, v, v_min, v_max);
        if (sliderHovered)
            trackCol = style::glass(56, 56, 56, style::glassWidget);

        if (leftArrow.Contains(ImGui::GetMousePos()) && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
            *v = std::max(*v - scale, v_min);
        if (rightArrow.Contains(ImGui::GetMousePos()) && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
            *v = std::min(*v + scale, v_max);
        if (sliderHovered && ImGui::IsKeyPressed(ImGuiKey_LeftArrow))
            *v = std::max(*v - scale, v_min);
        if (sliderHovered && ImGui::IsKeyPressed(ImGuiKey_RightArrow))
            *v = std::min(*v + scale, v_max);
    }

    *v = std::clamp(*v, v_min, v_max);
    const float interiorW = w - 2.f;
    const float fillW     = (*v - v_min) / (v_max - v_min) * interiorW;

    ImDrawList* dl = ImGui::GetWindowDrawList();

    if (hasLabel)
        dl->AddText(fonts::Verdana(), fonts::kUiSize, frameMin, style::col::text(), label);

    const ImVec2 inMin = sliderBb.Min + ImVec2(1.f, 1.f);
    const ImVec2 inMax(sliderBb.Max.x - 1.f, sliderBb.Max.y);
    dl->AddRectFilled(sliderBb.Min, sliderBb.Max + ImVec2(0.f, 1.f), style::col::widgetBorder());
    dl->AddRectFilled(inMin, inMax, trackCol);
    if (fillW > 0.f)
        dl->AddRectFilled(inMin, ImVec2(inMin.x + fillW, inMax.y), style::accentU32());

    constexpr float glyphSize = 9.f;
    const bool   leftH    = canInteract && leftArrow.Contains(ImGui::GetMousePos());
    const bool   rightH   = canInteract && rightArrow.Contains(ImGui::GetMousePos());
    const ImU32  dimC     = style::rgba(120, 120, 120);
    const ImU32  litC     = style::rgba(190, 190, 190);
    const ImVec2 minusSz  = fonts::Verdana()->CalcTextSizeA(glyphSize, FLT_MAX, 0.f, "-");
    const ImVec2 plusSz   = fonts::Verdana()->CalcTextSizeA(glyphSize, FLT_MAX, 0.f, "+");
    const float  centerY  = (sliderBb.Min.y + sliderBb.Max.y) * 0.5f;
    dl->AddText(fonts::Verdana(), glyphSize,
        ImVec2(sliderBb.Min.x - 9.f, centerY - minusSz.y * 0.5f), leftH ? litC : dimC, "-");
    dl->AddText(fonts::Verdana(), glyphSize,
        ImVec2(sliderBb.Max.x + 3.f, centerY - plusSz.y * 0.5f), rightH ? litC : dimC, "+");

    ImFont*      vFont    = fonts::VerdanaBold();
    const ImVec2 valueSz  = vFont->CalcTextSizeA(fonts::kUiSize, FLT_MAX, 0.f, formatted);
    const ImVec2 valuePos(inMin.x + fillW - valueSz.x * 0.5f, sliderBb.Min.y);
    dl->AddText(vFont, fonts::kUiSize, valuePos,
        style::rgba(255, 255, 255, std::min(230, style::menuAlpha)), formatted);

    ksd::AddItem(ImVec2(w + 20.f, labelH + 12.f));
}

namespace ksd {

void SliderInt(const char* label, int* v, int v_min, int v_max, const char* format, int remove) noexcept
{
    float fv = static_cast<float>(*v);
    DrawSliderImpl(label, v, &fv, static_cast<float>(v_min), static_cast<float>(v_max), format, 1.f, true, remove);
    *v = static_cast<int>(std::round(fv));
}

void SliderFloat(const char* label, float* v, float v_min, float v_max, const char* format, float scale) noexcept
{
    DrawSliderImpl(label, v, v, v_min, v_max, format, scale, false, 0);
}

}
