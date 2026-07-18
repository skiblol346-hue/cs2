#include "combo.hpp"
#include "../styling.hpp"
#include "../shell/shell.hpp"
#include "gui/assets/fonts.hpp"
#include "gui/assets/svg.hpp"
#include "imgui.h"
#include "imgui_internal.h"

#include <algorithm>
#include <cstdio>
#include <string>
#include <unordered_map>
#include <vector>

namespace
{
    struct ComboPopup
    {
        bool open = false;
    };

    struct PendingPopup
    {
        std::string         key;
        ImVec2              popPos   = {};
        ImVec2              popSize  = {};
        ImRect              frameBb;
        int*                current  = nullptr;
        const char* const*  items    = nullptr;
        const char* const*  icons    = nullptr;
        int                 count    = 0;
        float               w        = 0.f;
    };

    struct PendingMultiPopup
    {
        std::string         key;
        ImVec2              popPos  = {};
        ImVec2              popSize = {};
        ImRect              frameBb;
        int*                flags   = nullptr;
        const char* const*  items   = nullptr;
        int                 count   = 0;
        float               w       = 0.f;
    };

    struct ColorPickerState
    {
        bool   open       = false;
        float  h          = 0.f;
        float  s          = 0.f;
        float  v          = 0.f;
        float* col        = nullptr;
        ImVec2 anchorPos  = {};
        int    drag       = 0;
        int    framesOpen = 0;
    };

    std::unordered_map<std::string, ComboPopup>         g_combos;
    std::vector<PendingPopup>                           g_pendingPopups;
    std::vector<PendingMultiPopup>                      g_pendingMultiPopups;
    std::unordered_map<std::string, ColorPickerState>   g_colorPickers;
    std::vector<std::string>                            g_pendingColorPickers;

    void CloseAllCombosExcept(const char* keepKey) noexcept
    {
        for (auto& [key, popup] : g_combos)
        {
            if (!keepKey || key != keepKey)
                popup.open = false;
        }
        for (auto& [_, picker] : g_colorPickers)
            picker.open = false;
    }

    static void DrawCheckerboard(ImDrawList* dl, ImVec2 min, ImVec2 max, float cellPx) noexcept
    {
        const float w = max.x - min.x;
        const float h = max.y - min.y;
        const int   cols = std::max(1, static_cast<int>(w / cellPx));
        const int   rows = std::max(1, static_cast<int>(h / cellPx));
        const float cw = w / cols;
        const float ch = h / rows;
        for (int r = 0; r < rows; ++r)
        {
            for (int c = 0; c < cols; ++c)
            {
                const ImU32 col = ((r + c) % 2 == 0)
                    ? IM_COL32(160, 160, 160, 255)
                    : IM_COL32(96, 96, 96, 255);
                dl->AddRectFilled(
                    ImVec2(min.x + c * cw, min.y + r * ch),
                    ImVec2(min.x + (c + 1) * cw, min.y + (r + 1) * ch),
                    col);
            }
        }
    }

    bool ComboSelectable(ImDrawList* dl, const char* label, bool selected, const ImVec2& pos, const ImVec2& size, float textX = 10.f)
    {
        const ImRect bb(pos, pos + size);
        const bool   hovered = bb.Contains(ImGui::GetMousePos());
        const bool   pressed = hovered && ImGui::IsMouseReleased(ImGuiMouseButton_Left);

        if (hovered)
            dl->AddRectFilled(pos, pos + size, style::glass(40, 40, 40, style::glassWidget));

        const ImU32 textCol = selected ? style::accentU32() : style::col::text();
        dl->AddText(fonts::Verdana(), fonts::kUiSize,
            pos + ImVec2(textX, (size.y - fonts::CalcUiTextSize(label).y) * 0.5f),
            textCol, label);
        return pressed;
    }

    void DrawComboFrame(ImDrawList* dl, const ImVec2& frameMin, const ImVec2& frameSize,
                        const char* preview, bool hovered, bool open, bool borderless = false)
    {
        if (!borderless)
        {
            if (hovered || open)
                style::drawGradientV(dl, frameMin + ImVec2(1, 1), frameMin + frameSize - ImVec2(1, 1),
                    style::glass(41, 41, 41, style::glassWidget), style::glass(46, 46, 46, style::glassWidget));
            else
                style::drawGradientV(dl, frameMin + ImVec2(1, 1), frameMin + frameSize - ImVec2(1, 1),
                    style::glass(31, 31, 31, style::glassWidget), style::glass(36, 36, 36, style::glassWidget));

            dl->AddRect(frameMin, frameMin + frameSize, style::col::widgetBorder());
        }

        constexpr float kArrowInset = 14.f;
        ImFont* const pFont = fonts::Verdana();
        const float   pSize = borderless ? 13.f : fonts::kUiSize;
        const ImU32   pCol  = borderless ? style::rgba(225, 225, 225) : style::col::comboPreview();
        const float   pX    = borderless ? 6.f : 10.f;
        const float   clipR = borderless ? frameSize.x - 58.f : frameSize.x - kArrowInset;
        const ImVec2  previewSz = pFont->CalcTextSizeA(pSize, FLT_MAX, 0.f, preview);
        ImGui::PushClipRect(frameMin, frameMin + ImVec2(std::max(clipR, 20.f), frameSize.y), true);
        dl->AddText(pFont, pSize,
            frameMin + ImVec2(pX, (frameSize.y - previewSz.y) * 0.5f), pCol, preview);
        ImGui::PopClipRect();

        const float ax = frameSize.x - kArrowInset;
        dl->AddLine(frameMin + ImVec2(ax, 7), frameMin + ImVec2(ax + 5, 7), style::rgba(0, 0, 0));
        const ImVec2 tri0 = frameMin + ImVec2(ax, 8);
        dl->AddTriangleFilled(tri0, frameMin + ImVec2(ax + 2, 11), frameMin + ImVec2(ax + 5, 8),
            style::col::comboPreview());
    }
}

namespace ksd
{
    bool AnyComboOpen() noexcept
    {
        for (const auto& [_, popup] : g_combos)
            if (popup.open) return true;
        for (const auto& [_, picker] : g_colorPickers)
            if (picker.open) return true;
        return false;
    }

    void CloseAllPopups() noexcept
    {
        CloseAllCombosExcept(nullptr);
    }

    void RenderPendingPopups() noexcept
    {
        ImDrawList* dl = ImGui::GetForegroundDrawList();

        for (PendingPopup& pending : g_pendingPopups)
        {
            auto& popup = g_combos[pending.key];
            const ImRect popBb(pending.popPos, pending.popPos + pending.popSize);

            ksd::BlurRegion(pending.popPos, pending.popPos + pending.popSize);
            dl->AddRectFilled(pending.popPos, pending.popPos + pending.popSize, style::glass(18, 18, 22, style::glassWindow));
            dl->AddRectFilled(pending.popPos, pending.popPos + pending.popSize, style::col::panel());
            dl->AddRect(pending.popPos, pending.popPos + pending.popSize, style::col::border());

            const bool  hasIcons = pending.icons != nullptr;
            const float textX    = hasIcons ? 24.f : 10.f;
            const float step     = hasIcons ? 28.f : 20.f;
            const float rowH     = step - 2.f;
            for (int i = 0; i < pending.count; ++i)
            {
                const ImVec2 rowPos = pending.popPos + ImVec2(1, 1 + i * step);
                if (ComboSelectable(dl, pending.items[i], *pending.current == i,
                        rowPos, ImVec2(pending.w - 2, rowH), textX))
                {
                    *pending.current = i;
                    popup.open = false;
                }

                if (hasIcons)
                {
                    const ImVec2 sqMin(rowPos.x + 8.f, rowPos.y + (rowH - 7.f) * 0.5f);
                    dl->AddRectFilled(sqMin, sqMin + ImVec2(7, 7), style::accentU32());
                    dl->AddRect(sqMin - ImVec2(1, 1), sqMin + ImVec2(8, 8), style::col::widgetBorder());

                    if (pending.icons[i] && pending.icons[i][0])
                    {
                        int iw = 0, ih = 0;
                        const ImTextureID tex = svg::Get(pending.icons[i], 24, iw, ih);
                        if (tex)
                        {
                            const float ix = rowPos.x + (pending.w - 2.f) - iw - 6.f;
                            const float iy = rowPos.y + (rowH - ih) * 0.5f;
                            dl->AddImage(tex, ImVec2(ix, iy), ImVec2(ix + iw, iy + ih),
                                ImVec2(0.f, 0.f), ImVec2(1.f, 1.f), style::rgba(150, 150, 150));
                        }
                    }
                }
            }

            if (ImGui::IsMouseReleased(ImGuiMouseButton_Left)
                && !popBb.Contains(ImGui::GetMousePos())
                && !pending.frameBb.Contains(ImGui::GetMousePos()))
            {
                popup.open = false;
            }
        }
        g_pendingPopups.clear();

        for (PendingMultiPopup& pm : g_pendingMultiPopups)
        {
            auto& popup = g_combos[pm.key];
            const ImRect popBb(pm.popPos, pm.popPos + pm.popSize);

            ksd::BlurRegion(pm.popPos, pm.popPos + pm.popSize);
            dl->AddRectFilled(pm.popPos, pm.popPos + pm.popSize, style::glass(18, 18, 22, style::glassWindow));
            dl->AddRectFilled(pm.popPos, pm.popPos + pm.popSize, style::col::panel());
            dl->AddRect(pm.popPos, pm.popPos + pm.popSize, style::col::border());

            for (int i = 0; i < pm.count; ++i)
            {
                const ImVec2 rowPos = pm.popPos + ImVec2(1, 1 + i * 20.f);
                const ImVec2 rowSize(pm.w - 2, 18);
                const ImRect rowBb(rowPos, rowPos + rowSize);
                const bool   rowHov = rowBb.Contains(ImGui::GetMousePos());
                const bool   rowSel = (*pm.flags & (1 << i)) != 0;

                if (rowHov)
                    dl->AddRectFilled(rowPos, rowPos + rowSize, style::glass(40, 40, 40, style::glassWidget));

                dl->AddText(fonts::Verdana(), fonts::kUiSize,
                    rowPos + ImVec2(10, (rowSize.y - fonts::CalcUiTextSize(pm.items[i]).y) * 0.5f),
                    rowSel ? style::accentU32() : style::col::text(),
                    pm.items[i]);

                if (rowHov && ImGui::IsMouseReleased(ImGuiMouseButton_Left))
                    *pm.flags ^= (1 << i);
            }

            if (ImGui::IsMouseReleased(ImGuiMouseButton_Left)
                && !popBb.Contains(ImGui::GetMousePos())
                && !pm.frameBb.Contains(ImGui::GetMousePos()))
            {
                popup.open = false;
            }
        }
        g_pendingMultiPopups.clear();

        for (const std::string& pickId : g_pendingColorPickers)
        {
            auto it = g_colorPickers.find(pickId);
            if (it == g_colorPickers.end() || !it->second.open)
                continue;
            ColorPickerState& st = it->second;
            if (!st.col)
                continue;

            constexpr float kSvW   = 150.f;
            constexpr float kSvH   = 140.f;
            constexpr float kHueW  = 12.f;
            constexpr float kBarH  = 10.f;
            constexpr float kGap   = 4.f;
            constexpr float kPad   = 5.f;

            const float popW = kPad + kSvW + kGap + kHueW + kPad;
            const float popH = kPad + kSvH + kGap + kBarH + kGap + kBarH + kPad;

            const ImVec2 popMin = st.anchorPos;
            const ImVec2 popMax = popMin + ImVec2(popW, popH);
            const ImRect popBb(popMin, popMax);

            ksd::BlurRegion(popMin, popMax);
            dl->AddRectFilled(popMin, popMax, style::glass(18, 18, 22, style::glassWindow));
            dl->AddRectFilled(popMin, popMax, style::col::panel());
            dl->AddRect(popMin, popMax, style::col::border());

            const ImVec2 svMin(popMin.x + kPad, popMin.y + kPad);
            const ImVec2 svMax(svMin.x + kSvW, svMin.y + kSvH);
            const ImRect svBb(svMin, svMax);

            float hr, hg, hb;
            ImGui::ColorConvertHSVtoRGB(st.h, 1.f, 1.f, hr, hg, hb);
            const ImU32 hueCol = IM_COL32(
                static_cast<int>(hr * 255.f),
                static_cast<int>(hg * 255.f),
                static_cast<int>(hb * 255.f), 255);

            dl->AddRectFilledMultiColor(svMin, svMax,
                IM_COL32(255, 255, 255, 255), hueCol, hueCol, IM_COL32(255, 255, 255, 255));
            dl->AddRectFilledMultiColor(svMin, svMax,
                IM_COL32(0, 0, 0, 0), IM_COL32(0, 0, 0, 0),
                IM_COL32(0, 0, 0, 255), IM_COL32(0, 0, 0, 255));

            const ImVec2 svCursor(svMin.x + st.s * kSvW, svMin.y + (1.f - st.v) * kSvH);
            dl->AddCircle(svCursor, 4.f, IM_COL32(255, 255, 255, 200), 12, 1.5f);
            dl->AddCircle(svCursor, 4.f, IM_COL32(0,   0,   0,   120), 12, 0.5f);

            const ImVec2 hueMin(svMax.x + kGap, svMin.y);
            const ImVec2 hueMax(hueMin.x + kHueW, svMax.y);
            const ImRect hueBb(hueMin, hueMax);

            for (int i = 0; i < 6; ++i)
            {
                float r0, g0, b0, r1, g1, b1;
                ImGui::ColorConvertHSVtoRGB(static_cast<float>(i)     / 6.f, 1.f, 1.f, r0, g0, b0);
                ImGui::ColorConvertHSVtoRGB(static_cast<float>(i + 1) / 6.f, 1.f, 1.f, r1, g1, b1);
                const ImVec2 segMin(hueMin.x, hueMin.y + (i / 6.f) * kSvH);
                const ImVec2 segMax(hueMax.x, hueMin.y + ((i + 1) / 6.f) * kSvH);
                const ImU32 c0 = IM_COL32(
                    static_cast<int>(r0*255), static_cast<int>(g0*255), static_cast<int>(b0*255), 255);
                const ImU32 c1 = IM_COL32(
                    static_cast<int>(r1*255), static_cast<int>(g1*255), static_cast<int>(b1*255), 255);
                dl->AddRectFilledMultiColor(segMin, segMax, c0, c0, c1, c1);
            }

            const float hueY = hueMin.y + st.h * kSvH;
            dl->AddLine(ImVec2(hueMin.x, hueY), ImVec2(hueMax.x, hueY),
                IM_COL32(255, 255, 255, 230), 1.5f);

            const int rByte = static_cast<int>(st.col[0] * 255.f);
            const int gByte = static_cast<int>(st.col[1] * 255.f);
            const int bByte = static_cast<int>(st.col[2] * 255.f);
            const int aByte = static_cast<int>(st.col[3] * 255.f);

            const ImVec2 alphaMin(svMin.x, svMax.y + kGap);
            const ImVec2 alphaMax(svMax.x, alphaMin.y + kBarH);
            const ImRect alphaBb(alphaMin, alphaMax);

            DrawCheckerboard(dl, alphaMin, alphaMax, 5.f);

            const ImU32 colTransp = IM_COL32(rByte, gByte, bByte, 0);
            const ImU32 colOpaque = IM_COL32(rByte, gByte, bByte, 255);
            dl->AddRectFilledMultiColor(alphaMin, alphaMax,
                colTransp, colOpaque, colOpaque, colTransp);
            dl->AddRect(alphaMin, alphaMax, style::col::widgetBorder());

            const float alphaCurX = alphaMin.x + st.col[3] * (alphaMax.x - alphaMin.x);
            const ImVec2 handleTop(alphaCurX, alphaMin.y - 2.f);
            const ImVec2 handleBot(alphaCurX, alphaMax.y + 2.f);
            dl->AddLine(handleTop, handleBot, IM_COL32(0, 0, 0, 255), 3.f);
            dl->AddLine(handleTop, handleBot, IM_COL32(255, 255, 255, 255), 1.5f);

            const ImVec2 previewMin(svMin.x, alphaMax.y + kGap);
            const ImVec2 previewMax(svMax.x, previewMin.y + kBarH);
            DrawCheckerboard(dl, previewMin, previewMax, 5.f);
            dl->AddRectFilled(previewMin, previewMax, IM_COL32(rByte, gByte, bByte, aByte));
            dl->AddRect(previewMin, previewMax, style::col::widgetBorder());

            const ImVec2 mouse = ImGui::GetMousePos();
            const bool   down  = ImGui::IsMouseDown(ImGuiMouseButton_Left);

            if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
            {
                if (svBb.Contains(mouse))          st.drag = 1;
                else if (hueBb.Contains(mouse))    st.drag = 2;
                else if (alphaBb.Contains(mouse))  st.drag = 3;
                else if (!popBb.Contains(mouse) && st.framesOpen > 0)
                    st.open = false;
            }
            if (!down)
                st.drag = 0;

            if (down && st.drag == 1)
            {
                st.s = std::clamp((mouse.x - svMin.x) / kSvW, 0.f, 1.f);
                st.v = 1.f - std::clamp((mouse.y - svMin.y) / kSvH, 0.f, 1.f);
                ImGui::ColorConvertHSVtoRGB(st.h, st.s, st.v,
                    st.col[0], st.col[1], st.col[2]);
            }
            else if (down && st.drag == 2)
            {
                st.h = std::clamp((mouse.y - hueMin.y) / kSvH, 0.f, 1.f);
                ImGui::ColorConvertHSVtoRGB(st.h, st.s, st.v,
                    st.col[0], st.col[1], st.col[2]);
            }
            else if (down && st.drag == 3)
            {
                st.col[3] = std::clamp((mouse.x - alphaMin.x) / (alphaMax.x - alphaMin.x), 0.f, 1.f);
            }

            st.framesOpen++;
        }
        g_pendingColorPickers.clear();
    }

    void ColorButton(const char* id, float col[4], const ImVec2& pos, const ImVec2& size) noexcept
    {
        ImDrawList* dl = ImGui::GetWindowDrawList();
        const ImRect bb(pos, pos + size);

        DrawCheckerboard(dl, bb.Min, bb.Max, 4.f);
        dl->AddRectFilled(bb.Min, bb.Max,
            IM_COL32(static_cast<int>(col[0] * 255.f), static_cast<int>(col[1] * 255.f),
                     static_cast<int>(col[2] * 255.f), static_cast<int>(col[3] * 255.f)));
        style::drawGradientV(dl, bb.Min, bb.Max, IM_COL32(255, 255, 255, 30), IM_COL32(0, 0, 0, 30));
        dl->AddRect(bb.Min, bb.Max, style::col::widgetBorder());

        const bool hovered = bb.Contains(ImGui::GetMousePos()) && ksd::PopUpsAreClosed();
        if (hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
        {
            auto& st      = g_colorPickers[id];
            st.col        = col;
            st.anchorPos  = ImVec2(bb.Min.x - 160.f, bb.Max.y + 3.f);
            st.framesOpen = 0;
            ImGui::ColorConvertRGBtoHSV(col[0], col[1], col[2], st.h, st.s, st.v);
            CloseAllCombosExcept(nullptr);
            st.open = true;
        }

        auto it = g_colorPickers.find(id);
        if (it != g_colorPickers.end() && it->second.open)
            g_pendingColorPickers.push_back(id);
    }

    void Label(const char* label, bool special) noexcept
    {
        const ImVec2 pos       = ksd::CursorPos() + ImVec2(20, -3);
        const ImVec2 labelSize = ImGui::CalcTextSize(label);
        ImGui::GetWindowDrawList()->AddText(fonts::Verdana(), fonts::kUiSize, pos,
            special ? style::col::textSpecial() : style::col::text(), label);
        ksd::AddItem(ImVec2(labelSize.x + 20, labelSize.y));
    }

    bool SingleSelect(const char* label, int* current, const char* const* items, int count, float width, bool borderless, const char* const* icons) noexcept
    {
        if (count <= 0)
            return false;

        *current            = std::clamp(*current, 0, count - 1);
        const char* preview = items[*current];

        const ImVec2 base        = ksd::CursorPos();
        const float  w           = (width > 0.f) ? width : ksd::ContentWidth();
        const bool   hasLabel    = (label && label[0] != '\0');
        const ImVec2 labelSize   = hasLabel ? fonts::CalcUiTextSize(label) : ImVec2(0.f, 12.f);
        const float  labelH      = hasLabel ? labelSize.y : 0.f;
        const float  topOffset   = hasLabel ? -labelH / 3.f : -3.f;

        const ImVec2 fullMin(base.x + 20, base.y + topOffset);
        const float  fullSizeY   = labelH + 4.f + 20.f;
        const ImVec2 frameMin    = fullMin + ImVec2(0, labelH + 4.f);
        const ImVec2 frameSize(w, 20.f);
        const ImRect frameBb(frameMin, frameMin + frameSize);

        const bool hovered = frameBb.Contains(ImGui::GetMousePos());
        const bool clicked = hovered && ImGui::IsMouseReleased(ImGuiMouseButton_Left) && ksd::PopUpsAreClosed();

        char keyBuf[64];
        std::snprintf(keyBuf, sizeof(keyBuf), "combo_%s_%p", label, static_cast<const void*>(current));
        const std::string key   = keyBuf;
        auto&             popup = g_combos[key];

        if (clicked)
        {
            if (popup.open)
                popup.open = false;
            else
            {
                CloseAllCombosExcept(key.c_str());
                popup.open = true;
            }
        }

        ImDrawList* dl = ImGui::GetWindowDrawList();
        if (hasLabel)
            dl->AddText(fonts::Verdana(), fonts::kUiSize, fullMin + ImVec2(0, 1), style::col::text(), label);

        DrawComboFrame(dl, frameMin, frameSize, preview, hovered, popup.open, borderless);

        ksd::AddItem(ImVec2(std::max(labelSize.x, w + 20), fullSizeY), true);

        if (popup.open)
        {
            PendingPopup pending;
            pending.key     = key;
            pending.popPos  = fullMin + ImVec2(0, fullSizeY + 1);
            pending.popSize = ImVec2(w, (icons ? 28.f : 20.f) * count);
            pending.frameBb = frameBb;
            pending.current = current;
            pending.items   = items;
            pending.icons   = icons;
            pending.count   = count;
            pending.w       = w;
            g_pendingPopups.push_back(pending);
        }

        return true;
    }

    bool MultiSelect(const char* label, int* flags, const char* const* items, int count) noexcept
    {
        if (count <= 0)
            return false;

        char preview[64] = {};
        int  written     = 0;
        bool anySelected = false;
        for (int i = 0; i < count && written < 56; ++i)
        {
            if (!(*flags & (1 << i))) continue;
            if (anySelected)
            {
                preview[written++] = ',';
                preview[written++] = ' ';
            }
            const char* name = items[i];
            while (*name && written < 56)
                preview[written++] = *name++;
            anySelected = true;
        }
        if (!anySelected)
            std::snprintf(preview, sizeof(preview), "None");
        if (written >= 56)
        {
            preview[56] = '.'; preview[57] = '.'; preview[58] = '.'; preview[59] = '\0';
        }

        const ImVec2 base      = ksd::CursorPos();
        const float  w         = ksd::ContentWidth();
        const bool   hasLabel  = (label && label[0] != '\0');
        const ImVec2 labelSize = hasLabel ? fonts::CalcUiTextSize(label) : ImVec2(0.f, 12.f);
        const float  labelH    = hasLabel ? labelSize.y : 0.f;
        const float  topOffset = hasLabel ? -labelH / 3.f : -3.f;

        const ImVec2 fullMin(base.x + 20, base.y + topOffset);
        const float  fullSizeY = labelH + 4.f + 20.f;
        const ImVec2 frameMin  = fullMin + ImVec2(0, labelH + 4.f);
        const ImVec2 frameSize(w, 20.f);
        const ImRect frameBb(frameMin, frameMin + frameSize);

        const bool hovered = frameBb.Contains(ImGui::GetMousePos());
        const bool clicked = hovered && ImGui::IsMouseReleased(ImGuiMouseButton_Left) && ksd::PopUpsAreClosed();

        char keyBuf[64];
        std::snprintf(keyBuf, sizeof(keyBuf), "multi_%s_%p", label, static_cast<const void*>(flags));
        const std::string key   = keyBuf;
        auto&             popup = g_combos[key];

        if (clicked)
        {
            if (popup.open)
                popup.open = false;
            else
            {
                CloseAllCombosExcept(key.c_str());
                popup.open = true;
            }
        }

        ImDrawList* dl = ImGui::GetWindowDrawList();
        if (hasLabel)
            dl->AddText(fonts::Verdana(), fonts::kUiSize, fullMin + ImVec2(0, 1), style::col::text(), label);

        DrawComboFrame(dl, frameMin, frameSize, preview, hovered, popup.open);

        ksd::AddItem(ImVec2(std::max(labelSize.x, w + 20), fullSizeY), true);

        if (popup.open)
        {
            PendingMultiPopup pm;
            pm.key     = key;
            pm.popPos  = fullMin + ImVec2(0, fullSizeY + 1);
            pm.popSize = ImVec2(w, 18.f * count + count * 2.f);
            pm.frameBb = frameBb;
            pm.flags   = flags;
            pm.items   = items;
            pm.count   = count;
            pm.w       = w;
            g_pendingMultiPopups.push_back(pm);
        }

        return true;
    }
}
