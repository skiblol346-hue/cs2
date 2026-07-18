

#pragma once

// used: [stl] vector
#include <vector>
#include <unordered_map>
#include <string>
#include "../common.h"

// used: [ext] imgui, draw, animation
#include "../utilities/draw.h"


class CTab
{
public:
    const char* szName;
    void (*pRenderFunction)();
};
namespace T
{

    /* @section: values */

    inline char szConfigFile[256U] = {};

    inline unsigned long long nSelectedConfig = ~1U;

    inline int nCurrentOverlaySubtab = 0;
}
namespace MC
{
    static constexpr ImU32 BG_WINDOW = IM_COL32(0x0d, 0x0d, 0x0d, 0xFF);
    static constexpr ImU32 BG_SIDEBAR = IM_COL32(0x0a, 0x0a, 0x0a, 0xFF);
    static constexpr ImU32 BG_PANEL = IM_COL32(0x12, 0x12, 0x12, 0xFF);
    static constexpr ImU32 BORDER = IM_COL32(0x22, 0x22, 0x22, 0xFF);
    static constexpr ImU32 BORDER_DIM = IM_COL32(0x1a, 0x1a, 0x1a, 0xB4);
    static constexpr ImU32 ACCENT = IM_COL32(0x78, 0x78, 0x78, 0xFF);
    static constexpr ImU32 ACCENT_HI = IM_COL32(0xaa, 0xaa, 0xaa, 0xFF);
    static constexpr ImU32 ACCENT2 = IM_COL32(0x55, 0x55, 0x55, 0xFF);
    static constexpr ImU32 GRAD_A = IM_COL32(0x70, 0x70, 0x70, 0xFF);
    static constexpr ImU32 GRAD_B = IM_COL32(0x18, 0x18, 0x18, 0xFF);
    static constexpr ImU32 TEXT = IM_COL32(0xd8, 0xd8, 0xd8, 0xFF);
    static constexpr ImU32 TEXT_DIM = IM_COL32(0x72, 0x72, 0x72, 0xFF);
    static constexpr ImU32 TEXT_MUTED = IM_COL32(0x44, 0x44, 0x44, 0xFF);
    static constexpr ImU32 TEXT_DARK = IM_COL32(0x28, 0x28, 0x28, 0xFF);
    static constexpr ImU32 RED = IM_COL32(0xcc, 0x33, 0x33, 0xFF);
    static constexpr ImU32 RED_LINE = IM_COL32(0xcc, 0x33, 0x33, 0x33);
    static constexpr ImU32 INPUT_BG = IM_COL32(0x10, 0x10, 0x10, 0xFF);
    static constexpr ImU32 INPUT_BDR = IM_COL32(0x22, 0x22, 0x22, 0xFF);
    static constexpr ImU32 TOGGLE_OFF = IM_COL32(0x20, 0x20, 0x20, 0xFF);

    inline ImVec4 V4(ImU32 c) {
        return ImVec4(
            ((c >> IM_COL32_R_SHIFT) & 0xFF) / 255.f,
            ((c >> IM_COL32_G_SHIFT) & 0xFF) / 255.f,
            ((c >> IM_COL32_B_SHIFT) & 0xFF) / 255.f,
            ((c >> IM_COL32_A_SHIFT) & 0xFF) / 255.f);
    }

    inline ImU32 LerpU32(ImU32 a, ImU32 b, float t) {
        auto ch = [&](int sh) -> int {
            int ca = (a >> sh) & 0xFF;
            int cb = (b >> sh) & 0xFF;
            return (int)(ca + (cb - ca) * t);
            };
        return IM_COL32(ch(IM_COL32_R_SHIFT), ch(IM_COL32_G_SHIFT),
            ch(IM_COL32_B_SHIFT), 0xFF);
    }

    inline ImU32 ColToU32(const Color_t& c) {
        return IM_COL32(c.r, c.g, c.b, c.a);
    }
}

namespace MENU
{
    void RenderMainWindow();
    void UpdateStyle(ImGuiStyle* pStyle = nullptr);
    inline float flDpiScale = 1.f;
    inline bool bMainWindowOpened = false;
    inline bool bMainActive = false;
    inline AnimationHandler_t animMenuDimBackground;
    inline int nCurrentMainTab = 0;

}

