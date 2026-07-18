#include "shell.hpp"
#include "dx_blur.h"
#include "../combo/combo.hpp"
#include "../keybind/keybind.hpp"
#include "../styling.hpp"
#include "gui/gui.hpp"
#include "gui/assets/fonts.hpp"
#include "gui/assets/svg.hpp"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <vector>

static constexpr float kMenuW        = 660.f;
static constexpr float kMenuH        = 560.f;
static constexpr float kSidebarW     = 80.f;
static constexpr float kContentX     = 100.f;
static constexpr float kContentY     = 30.f;
static constexpr float kContentPadR  = 23.f;
static constexpr float kContentPadB  = 23.f;
static constexpr float kItemSpacingY = 4.f;
static constexpr float kRollDuration = 0.12f;
static constexpr float kAccentFlowSpeed = 1.70f;
static constexpr float kAccentFlowWaves = 0.80f;
static constexpr float kAccentFlowFloor = 0.40f;
static constexpr float kTabGlowDuration = 0.10f;

struct ShellState
{
    ImVec2 windowPos  = {};
    ImVec2 windowSize = { kMenuW, kMenuH };
    int    currentTab = 0;
    int    prevTab    = -1;

    float  rollProgress    = 1.f;
    float  rollStartOffset = 0.f;
    float  contentOffsetY  = 0.f;
    bool   contentClipPushed = false;

    ksd::ChildState*  activeChild = nullptr;
    std::unordered_map<std::string, ksd::ChildState> children;
    std::vector<ksd::ChildState*> frameChildren;

    std::string selectedItem;
    std::unordered_map<std::string, bool> itemActive;
    std::unordered_map<int, float> tabGlowProgress;
    bool blockInput = false;
    bool menuDragActive  = false;
    ImVec2 menuDragOffset = {};

    std::string scrollDragId;
    float scrollDragGrab = 0.f;

    ImVec2 menuPos     = {};
    bool   menuPosInit = false;
};

static ShellState g;

static std::string ChildDisplayName(const char* name)
{
    std::string s(name);
    const auto hash = s.find('#');
    if (hash != std::string::npos)
        s = s.substr(0, hash);
    return s;
}

static void RefreshWindowRect() noexcept
{
    g.windowPos  = ImGui::GetWindowPos();
    g.windowSize = ImGui::GetWindowSize();
}

static void ComputeChildLayout(ksd::ChildState& c)
{
    const ImVec2 boundsMin = g.windowPos
        + ImVec2(kContentX - 1.f, kContentY - 1.f + g.contentOffsetY);
    const ImVec2 boundsSize = ImVec2(
        g.windowSize.x - kContentX - kContentPadR,
        g.windowSize.y - kContentY - kContentPadB);

    constexpr float kGap = 5.f;
    const float colW = (boundsSize.x - 2.f) / 9.f;
    const float rowH = (boundsSize.y - 2.f) / 10.f;

    c.pos  = boundsMin + ImVec2(c.xPos * colW + kGap * 0.5f, c.yPos * rowH + kGap * 0.5f);
    c.size = ImVec2(
        std::max(c.xSize * colW - kGap, 30.f),
        std::max(c.ySize * rowH - kGap, 40.f));
}

static bool IsInside(const ImVec2& p, const ImVec2& min, const ImVec2& size) noexcept
{
    return p.x > min.x && p.y > min.y
        && p.x < min.x + size.x && p.y < min.y + size.y;
}

static bool IsMouseOverChildWidgetArea(const ImVec2& mouse) noexcept
{
    for (const ksd::ChildState* child : g.frameChildren)
    {
        const ImVec2 clipMin(child->pos.x + 2.f,                child->pos.y + 4.f);
        const ImVec2 clipMax(child->pos.x + child->size.x - 15.f, child->pos.y + child->size.y - 3.f);
        if (mouse.x > clipMin.x && mouse.y > clipMin.y &&
            mouse.x < clipMax.x && mouse.y < clipMax.y)
            return true;
    }
    return false;
}

static bool IsMouseOverSidebarTab(const ImVec2& mouse) noexcept
{
    return mouse.x >= g.windowPos.x && mouse.x < g.windowPos.x + kSidebarW;
}

static bool IsMenuDragZone(const ImVec2& mouse) noexcept
{
    const ImRect topBar(
        g.windowPos + ImVec2(kSidebarW, 0.f),
        g.windowPos + ImVec2(g.windowSize.x, kContentY));
    if (topBar.Contains(mouse))
        return true;

    const ImRect content(
        g.windowPos + ImVec2(kContentX - 1.f, kContentY - 1.f),
        g.windowPos + ImVec2(g.windowSize.x - kContentPadR, g.windowSize.y - kContentPadB));
    if (!content.Contains(mouse))
        return false;
    return !IsMouseOverChildWidgetArea(mouse);
}

static void HandleMainWindowDrag() noexcept
{
    if (g.blockInput || !g.selectedItem.empty() || ksd::AnyComboOpen())
    {
        g.menuDragActive = false;
        return;
    }
    const ImVec2 mouse = ImGui::GetMousePos();
    if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
    {
        if (IsMenuDragZone(mouse) && !IsMouseOverSidebarTab(mouse))
        {
            g.menuDragActive = true;
            g.menuDragOffset = mouse - g.windowPos;
        }
        else
        {
            g.menuDragActive = false;
        }
    }
    if (!g.menuDragActive)
        return;
    if (!ImGui::IsMouseDown(ImGuiMouseButton_Left))
    {
        g.menuDragActive = false;
        return;
    }
    g.menuPos = mouse - g.menuDragOffset;
}

static void DrawTopAccentFlow(ImDrawList* dl, ImVec2 barMin, ImVec2 barMax)
{
    const float width = barMax.x - barMin.x;
    if (width <= 1.f)
        return;

    const float accentR = vars::colorAccent[0];
    const float accentG = vars::colorAccent[1];
    const float accentB = vars::colorAccent[2];

    const float scroll = static_cast<float>(ImGui::GetTime()) * kAccentFlowSpeed;

    const int   segments = 64;
    const float segW = width / static_cast<float>(segments);

    auto sampleColor = [&](int index) -> ImU32
    {
        const float u = static_cast<float>(index) / static_cast<float>(segments);
        const float wave  = 0.5f + 0.5f * std::sin(6.2831853f * kAccentFlowWaves * (u - scroll));
        const float level = kAccentFlowFloor + (1.f - kAccentFlowFloor) * wave;
        return style::rgba(
            static_cast<int>(accentR * 255.f * level),
            static_cast<int>(accentG * 255.f * level),
            static_cast<int>(accentB * 255.f * level));
    };

    for (int i = 0; i < segments; ++i)
    {
        const ImVec2 segMin(barMin.x + segW * static_cast<float>(i),     barMin.y);
        const ImVec2 segMax(barMin.x + segW * static_cast<float>(i + 1), barMax.y);
        style::drawGradientH(dl, segMin, segMax, sampleColor(i), sampleColor(i + 1));
    }
}

static void DrawTabGlow(ImDrawList* dl, ImVec2 center, float progress)
{
    const float eased = progress * progress * (3.f - 2.f * progress);
    const int   perLayer = static_cast<int>(11.f * eased * (static_cast<float>(style::menuAlpha) / 255.f));
    if (perLayer <= 0)
        return;

    const int ar = static_cast<int>(vars::colorAccent[0] * 255.f);
    const int ag = static_cast<int>(vars::colorAccent[1] * 255.f);
    const int ab = static_cast<int>(vars::colorAccent[2] * 255.f);

    const int   layers    = 16;
    const float maxRadius = 26.f * (0.4f + 0.6f * eased);
    for (int i = layers; i >= 1; --i)
    {
        const float r = maxRadius * (static_cast<float>(i) / static_cast<float>(layers));
        dl->AddCircleFilled(center, r, IM_COL32(ar, ag, ab, perLayer), 28);
    }
}

static void DrawMainChrome(ImDrawList* dl, ImVec2 pos, ImVec2 size)
{
    const float  topY = 6.f;
    const ImVec2 top(pos.x, pos.y + topY);

    if (g_swapChain)
        draw_background_blur(dl, g_swapChain, device, ctx, top, pos + size, 0);

    dl->AddRectFilled(top, pos + size, style::glass(18, 18, 22, style::glassWindow));
    dl->AddRectFilled(top, ImVec2(pos.x + kSidebarW, pos.y + size.y), style::glass(14, 14, 18, 0.20f));
    dl->AddLine(
        pos + ImVec2(kSidebarW, 10.f),
        pos + ImVec2(kSidebarW, size.y),
        style::col::separator());

    const ImU32 frame = style::col::outline();
    dl->AddRectFilled(top,                              pos + ImVec2(2.f, size.y),          frame);
    dl->AddRectFilled(pos + ImVec2(size.x - 2.f, topY), pos + size,                         frame);
    dl->AddRectFilled(pos + ImVec2(2.f, size.y - 2.f),  pos + ImVec2(size.x - 2.f, size.y), frame);

    DrawTopAccentFlow(dl,
        pos + ImVec2(2.f, topY), pos + ImVec2(size.x - 2.f, topY + 4.f));
}

static void DrawChildPanelFill(ImDrawList* dl, const ksd::ChildState& c)
{
    dl->AddRectFilled(c.pos, c.pos + c.size, style::col::panel());
}

static void DrawChildBorderAndTitle(ImDrawList* dl, const ksd::ChildState& c)
{
    const ImVec2 p = c.pos;
    const ImVec2 s = c.size;
    const ImU32  outer = style::col::widgetBorder();
    const ImU32  inner = style::col::childBorder(c.resizing);

    float labelW = 0.f;
    if (!c.displayName.empty())
        labelW = fonts::VerdanaBold()->CalcTextSizeA(
            fonts::kUiSize, FLT_MAX, 0.f, c.displayName.c_str()).x;

    const float gapStart = 10.f;
    const float gapEnd   = 16.f + labelW;

    dl->AddLine(p,                          p + ImVec2(0.f,        s.y),        outer);
    dl->AddLine(p + ImVec2(0.f, s.y - 1.f), p + ImVec2(s.x,        s.y - 1.f), outer);
    dl->AddLine(p + ImVec2(s.x - 1.f, 0.f), p + ImVec2(s.x - 1.f, s.y),        outer);
    dl->AddLine(p,                          p + ImVec2(gapStart,   0.f),        outer);
    dl->AddLine(p + ImVec2(gapEnd, 0.f),    p + ImVec2(s.x,        0.f),        outer);

    dl->AddLine(p + ImVec2(1.f, 1.f),        p + ImVec2(1.f,        s.y - 1.f), inner);
    dl->AddLine(p + ImVec2(1.f, s.y - 2.f),  p + ImVec2(s.x - 1.f,  s.y - 2.f), inner);
    dl->AddLine(p + ImVec2(s.x - 2.f, 1.f),  p + ImVec2(s.x - 2.f,  s.y - 1.f), inner);
    dl->AddLine(p + ImVec2(1.f, 1.f),        p + ImVec2(gapStart,   1.f),       inner);
    dl->AddLine(p + ImVec2(gapEnd, 1.f),     p + ImVec2(s.x - 1.f,  1.f),       inner);

    if (!c.displayName.empty())
    {
        const float ty = p.y - fonts::kUiSize * 0.5f;
        dl->AddText(fonts::VerdanaBold(), fonts::kUiSize,
            ImVec2(p.x + 14.f, ty + 1.f), style::rgba(15, 15, 15), c.displayName.c_str());
        dl->AddText(fonts::VerdanaBold(), fonts::kUiSize,
            ImVec2(p.x + 13.f, ty),       style::col::textTitle(),  c.displayName.c_str());
    }
}

static void UpdateRollAnimation() noexcept
{
    if (g.rollProgress >= 1.f)
    {
        g.contentOffsetY = 0.f;
        return;
    }
    const float dt = ImGui::GetIO().DeltaTime;
    g.rollProgress = std::min(g.rollProgress + dt / kRollDuration, 1.f);
    const float t     = g.rollProgress;
    const float eased = 1.f - (1.f - t) * (1.f - t);
    g.contentOffsetY  = g.rollStartOffset * (1.f - eased);
}

static void ApplyChildScroll(ksd::ChildState& c) noexcept
{
    const ImVec2 base = c.pos + ImVec2(22.f, 23.f);
    c.cursor        = base;
    c.appliedScroll = 0.f;
    if (c.scrollRatio < 0.f || c.contentHeight <= c.size.y + 1.f)
        return;
    const float overflow = c.contentHeight - c.size.y;
    const float offset   = overflow * std::clamp(c.scrollRatio, 0.f, 1.f);
    c.cursor.y      = base.y - offset;
    c.appliedScroll = offset;
}

namespace ksd {

void ResetChildScrollStates() noexcept
{
    for (auto& [_, child] : g.children)
    {
        child.scrollRatio    = -1.f;
        child.contentBottomY = 0.f;
    }
}

void BeginMain() noexcept
{
    if (!g.menuPosInit)
    {
        const ImVec2 center = ImGui::GetMainViewport()->GetCenter();
        g.menuPos     = center - ImVec2(kMenuW * 0.5f, kMenuH * 0.5f);
        g.menuPosInit = true;
    }

    if (g.prevTab != -1 && g.currentTab != g.prevTab)
    {
        const int dir     = (g.currentTab > g.prevTab) ? 1 : -1;
        g.rollStartOffset = static_cast<float>(dir) * kMenuH;
        g.rollProgress    = 0.f;
        g.contentOffsetY  = g.rollStartOffset;

        ksd::CloseAllPopups();
        g.selectedItem.clear();
        g.itemActive.clear();
    }
    g.prevTab = g.currentTab;

    if (!ImGui::IsMouseDown(ImGuiMouseButton_Left))
        g.selectedItem.clear();

    UpdateRollAnimation();
    ksd::ProcessKeybinds();

    HandleMainWindowDrag();

    ImGui::SetNextWindowSize(ImVec2(kMenuW, kMenuH), ImGuiCond_Always);
    ImGui::SetNextWindowPos(g.menuPos,               ImGuiCond_Always);

    constexpr ImGuiWindowFlags flags =
        ImGuiWindowFlags_NoTitleBar        |
        ImGuiWindowFlags_NoMove            |
        ImGuiWindowFlags_NoResize          |
        ImGuiWindowFlags_NoScrollbar       |
        ImGuiWindowFlags_NoScrollWithMouse |
        ImGuiWindowFlags_NoCollapse;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding,    ImVec2(0.f, 0.f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.f);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.f, 0.f, 0.f, 0.f));
    ImGui::Begin("##SkeetMain", nullptr, flags);

    RefreshWindowRect();
    g.blockInput        = false;
    g.frameChildren.clear();
    g.contentClipPushed = false;

    DrawMainChrome(ImGui::GetWindowDrawList(), g.windowPos, g.windowSize);
}

void EndMain() noexcept
{
    if (g.contentClipPushed)
    {
        ImGui::PopClipRect();
        ImGui::GetWindowDrawList()->PopClipRect();
        g.contentClipPushed = false;
    }
    ImGui::End();
    ImGui::PopStyleColor();
    ImGui::PopStyleVar(2);
    g.activeChild = nullptr;
}

void BlurRegion(const ImVec2& min, const ImVec2& max) noexcept
{
    if (!g_swapChain)
        return;
    draw_background_blur(ImGui::GetForegroundDrawList(), g_swapChain, device, ctx, min, max, 0);
}

bool TabButton(const char* label, int* selected, int num, int total, bool legitTab, float slotWidthPx) noexcept
{
    ImDrawList* dl = ImGui::GetWindowDrawList();

    if (legitTab)
    {
        const float panelW = g.activeChild ? g.activeChild->size.x : 535.f;
        const float panelH = g.activeChild ? g.activeChild->size.y : 66.f;
        const ImVec2 base  = g.activeChild ? g.activeChild->pos : g.windowPos;

        const bool  grouped = slotWidthPx > 0.f;
        const float slotW   = grouped ? slotWidthPx : panelW / static_cast<float>(total);
        const float xOff    = grouped ? 10.f : 0.f;

        const int    idx     = std::min(num, total - 1);
        const ImVec2 slotPos = base + ImVec2(xOff + slotW * static_cast<float>(idx), 0.f);
        const ImVec2 sz(slotW, panelH);
        const ImRect bb(slotPos, slotPos + sz);

        const bool hovered = bb.Contains(ImGui::GetMousePos());
        const bool pressed = hovered && ImGui::IsMouseReleased(ImGuiMouseButton_Left);
        if (pressed) *selected = num;

        const ImU32 col = (*selected == num) ? style::col::tabActive()
                        : hovered            ? style::col::tabHover()
                        :                      style::col::tabIdle();

        const int iconPx = grouped ? 38 : 28;
        int iconW = 0, iconH = 0;
        const ImTextureID iconTex = svg::Get(label, iconPx, iconW, iconH);
        if (iconTex)
        {
            const float ix = slotPos.x + (sz.x - static_cast<float>(iconW)) * 0.5f;
            const float iy = slotPos.y + (sz.y - static_cast<float>(iconH)) * 0.5f;
            dl->AddImage(iconTex, ImVec2(ix, iy), ImVec2(ix + iconW, iy + iconH),
                ImVec2(0.f, 0.f), ImVec2(1.f, 1.f), col);
        }
        return pressed;
    }

    constexpr float kSidebarTop = 19.f;
    constexpr float kSidebarBot = 6.f;
    const int   tabCount = total + 1;
    const float slotH    = (g.windowSize.y - kSidebarTop - kSidebarBot) / static_cast<float>(tabCount);
    const ImVec2 slotPos = g.windowPos + ImVec2(0.f, kSidebarTop + slotH * static_cast<float>(num));
    const ImRect bb(slotPos, slotPos + ImVec2(kSidebarW, slotH));

    const bool hovered = bb.Contains(ImGui::GetMousePos());
    const bool pressed = hovered && ImGui::IsMouseReleased(ImGuiMouseButton_Left) && !g.blockInput;
    if (pressed) *selected = num;

    const bool isActive = (*selected == num);

    const ImVec2 iconCenter = slotPos + ImVec2(kSidebarW * 0.5f, slotH * 0.5f);

    float&      glow   = g.tabGlowProgress[num];
    const float step   = (kTabGlowDuration > 0.f) ? ImGui::GetIO().DeltaTime / kTabGlowDuration : 1.f;
    const float target = isActive ? 1.f : 0.f;
    if (glow < target)
        glow = std::min(glow + step, target);
    else if (glow > target)
        glow = std::max(glow - step, target);

    if (glow > 0.001f)
        DrawTabGlow(dl, iconCenter, glow);

    ImU32 iconCol;
    if (isActive)
        iconCol = style::accentU32();
    else if (hovered)
        iconCol = style::accentU32(static_cast<int>(115.f * static_cast<float>(style::menuAlpha) / 255.f));
    else
        iconCol = style::col::tabIdle();

    ImFont*      font = fonts::TabIcons();
    const ImVec2 ts   = fonts::CalcTabIconSize(label);
    dl->AddText(font, fonts::kTabIconSize,
        slotPos + ImVec2(kSidebarW * 0.5f - ts.x * 0.5f + 2.f, slotH * 0.5f - ts.y * 0.5f),
        iconCol, label);

    return pressed;
}

bool BeginChild(const char* name, GridRect grid, bool) noexcept
{
    ksd::ChildState& c = g.children[name];
    if (!c.init)
    {
        c.name        = name;
        c.displayName = ChildDisplayName(name);
        c.init        = true;
    }

    c.xPos    = grid.x;
    c.yPos    = grid.y;
    c.xSize   = grid.w;
    c.ySize   = grid.h;
    c.dragging  = false;
    c.resizing  = false;

    RefreshWindowRect();
    ComputeChildLayout(c);
    g.frameChildren.push_back(&c);

    if (!g.contentClipPushed)
    {
        const ImVec2 cMin(g.windowPos.x + kContentX - 1.f,
                          g.windowPos.y + kContentY - 14.f);
        const ImVec2 cMax(g.windowPos.x + g.windowSize.x - kContentPadR,
                          g.windowPos.y + g.windowSize.y - kContentPadB);
        ImGui::GetWindowDrawList()->PushClipRect(cMin, cMax, true);
        ImGui::PushClipRect(cMin, cMax, true);
        g.contentClipPushed = true;
    }

    DrawChildPanelFill(ImGui::GetWindowDrawList(), c);

    g.activeChild = &c;
    ApplyChildScroll(c);

    const ImVec2 clipMin(c.pos.x + 2.f,          c.pos.y + 4.f);
    const ImVec2 clipMax(c.pos.x + c.size.x - 15.f, c.pos.y + c.size.y - 3.f);
    ImGui::PushClipRect(clipMin, clipMax, true);
    return true;
}

void EndChild() noexcept
{
    if (!g.activeChild)
        return;

    ksd::ChildState& c = *g.activeChild;
    c.prevCursor   = c.cursor;
    c.prevCursor.y += 8.f;
    const float contentBottom = c.prevCursor.y + c.appliedScroll;
    c.contentBottomY = contentBottom;
    c.contentHeight  = contentBottom - c.pos.y;

    const float maxY       = c.pos.y + c.size.y;
    const bool  needsScroll = contentBottom > maxY;
    if (!needsScroll)
        c.scrollRatio = -1.f;

    ImGui::PopClipRect();
    ksd::RenderPendingPopups();

    RefreshWindowRect();
    ComputeChildLayout(c);

    if (needsScroll)
    {
        if (c.scrollRatio < 0.f)
            c.scrollRatio = 0.f;

        const float overflow = contentBottom - maxY;
        const float scrollH  = c.size.y / ((contentBottom - c.pos.y) / c.size.y);
        const float range    = c.size.y - scrollH;
        const float trackTop = c.pos.y + 3.f;
        const float thumbH   = scrollH - 6.f;
        const ImVec2 mouse   = ImGui::GetMousePos();

        const float  thumbTop = trackTop + range * c.scrollRatio;
        const ImRect trackBb(
            ImVec2(c.pos.x + c.size.x - 7.f, c.pos.y + 1.f),
            ImVec2(c.pos.x + c.size.x - 1.f, c.pos.y + c.size.y - 1.f));
        const ImRect thumbBb(
            ImVec2(c.pos.x + c.size.x - 6.f, thumbTop),
            ImVec2(c.pos.x + c.size.x - 2.f, thumbTop + thumbH));

        if (g.scrollDragId.empty() && ImGui::IsMouseClicked(ImGuiMouseButton_Left) && trackBb.Contains(mouse))
        {
            g.scrollDragGrab = thumbBb.Contains(mouse) ? (mouse.y - thumbTop) : (thumbH * 0.5f);
            g.scrollDragId   = c.name;
            g.menuDragActive = false;
        }
        if (g.scrollDragId == c.name)
        {
            if (ImGui::IsMouseDown(ImGuiMouseButton_Left) && range > 0.f)
                c.scrollRatio = std::clamp((mouse.y - g.scrollDragGrab - trackTop) / range, 0.f, 1.f);
            else
                g.scrollDragId.clear();
        }

        const float offset = range * c.scrollRatio;

        ImDrawList* dl = ImGui::GetWindowDrawList();
        dl->AddRectFilled(
            ImVec2(c.pos.x + c.size.x - 7.f, c.pos.y + 1.f),
            ImVec2(c.pos.x + c.size.x - 1.f, c.pos.y + c.size.y - 1.f),
            style::col::gray40());
        dl->AddRectFilled(
            ImVec2(c.pos.x + c.size.x - 6.f, trackTop + offset),
            ImVec2(c.pos.x + c.size.x - 2.f, trackTop + offset + thumbH),
            style::col::gray65());

        if (IsInside(mouse, c.pos, c.size) && ImGui::GetIO().MouseWheel != 0.f)
        {
            c.scrollRatio = std::clamp(
                c.scrollRatio - ImGui::GetIO().MouseWheel * (20.f / overflow), 0.f, 1.f);
        }
    }

    DrawChildBorderAndTitle(ImGui::GetWindowDrawList(), c);
    g.activeChild = nullptr;
}

ImVec2 CursorPos() noexcept
{
    return g.activeChild ? g.activeChild->cursor : ImGui::GetWindowPos();
}

void AddItem(const ImVec2& size, bool comboSpacing) noexcept
{
    if (!g.activeChild) return;
    ksd::ChildState& c = *g.activeChild;
    c.prevCursor = c.cursor;
    c.cursor.y  += size.y + (comboSpacing ? 2.f : kItemSpacingY);
}

float ContentWidth() noexcept
{
    if (!g.activeChild) return 200.f;
    return std::clamp(g.activeChild->size.x - 90.f, 63.f, 202.f);
}

ImVec2 WindowPos()  noexcept { return g.windowPos; }
ImVec2 WindowSize() noexcept { return g.windowSize; }

float PanelRightX() noexcept
{
    if (!g.activeChild) return 0.f;
    return g.activeChild->pos.x + g.activeChild->size.x - 17.f;
}

bool IsPopupBlocking()                     noexcept { return !g.selectedItem.empty(); }
bool ItemActive(const char* id)            noexcept { return g.itemActive[id]; }
void SetItemActive(const char* id, bool v) noexcept { g.itemActive[id] = v; }
const char* SelectedItem()                 noexcept { return g.selectedItem.c_str(); }
void SetSelectedItem(const char* id)       noexcept { g.selectedItem = id ? id : ""; }
bool PopUpsAreClosed()                     noexcept { return !ksd::AnyComboOpen(); }
int& CurrentTab()                          noexcept { return g.currentTab; }

bool ButtonBehavior(const char* id, ImRect bb, bool returnOnRelease) noexcept
{
    const bool hovered = bb.Contains(ImGui::GetMousePos());
    const bool clicked = returnOnRelease
        ? (hovered && ImGui::IsMouseReleased(ImGuiMouseButton_Left))
        : (hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left));

    if (!ksd::PopUpsAreClosed() || (!g.selectedItem.empty() && g.selectedItem != id))
        return false;

    if (hovered && clicked && !ItemActive(id) && g.selectedItem.empty())
    {
        SetItemActive(id, true);
        SetSelectedItem(id);
        return true;
    }

    if (!ImGui::IsMouseDown(ImGuiMouseButton_Left) && ItemActive(id))
    {
        SetItemActive(id, false);
        SetSelectedItem("");
    }

    return hovered && clicked;
}

}
