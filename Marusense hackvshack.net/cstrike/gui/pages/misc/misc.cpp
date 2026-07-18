#include "misc.hpp"
#include "../../framework/shell/shell.hpp"
#include "../../framework/keybind/keybind.hpp"
#include <Windows.h>
#include "core/variables.h"

namespace pages
{
    void RenderMisc() noexcept
    {
        if (ksd::BeginChild("Miscellaneous", { 0, 0, 4.5f, 6 }))
        {
            ksd::Checkbox("Anti-flash", &C_GET(bool, Vars.bAntiFlash));
            ksd::Checkbox("Anti-smoke", &C_GET(bool, Vars.bAntiSmoke));
            ksd::Checkbox("Bunny hop",  &C_GET(bool, Vars.bBhop));
            ksd::Checkbox("Auto-accept", &C_GET(bool, Vars.bAutoAccept));
            ksd::Checkbox("Last second defuse", &C_GET(bool, Vars.bLastSecondDefuse));
            ksd::Checkbox("Radar hack", &C_GET(bool, Vars.bRadarHack));
            ksd::Checkbox("Clantag",    &C_GET(bool, Vars.bClantag));
        }
        ksd::EndChild();

        if (ksd::BeginChild("Movement", { 0, 6, 4.5f, 4 }))
        {
            ksd::Checkbox("Edge bug", &C_GET(bool, Vars.bEdgeBug));
            if (C_GET(bool, Vars.bEdgeBug))
            {
                const float rowY = ksd::CursorPos().y;
                ksd::Label("Edge bug key");
                static int ebKey = static_cast<int>(C_GET(unsigned int, Vars.nEdgeBugKey));
                ksd::DrawKeybindButton("##ebkey", &ebKey, nullptr, rowY);
                C_GET(unsigned int, Vars.nEdgeBugKey) = static_cast<unsigned int>(ebKey);

                ksd::SliderInt("Prediction ticks", &C_GET(int, Vars.nEdgeBugTicks), 1, 64, "%d");
            }

            ksd::Checkbox("Jump bug", &C_GET(bool, Vars.bJumpBug));
            if (C_GET(bool, Vars.bJumpBug))
            {
                const float rowY = ksd::CursorPos().y;
                ksd::Label("Jump bug key");
                static int jbKey = static_cast<int>(C_GET(unsigned int, Vars.nJumpBugKey));
                ksd::DrawKeybindButton("##jbkey", &jbKey, nullptr, rowY);
                C_GET(unsigned int, Vars.nJumpBugKey) = static_cast<unsigned int>(jbKey);
            }
        }
        ksd::EndChild();

        if (ksd::BeginChild("Viewmodel", { 4.5f, 0, 4.5f, 5 }))
        {
            ksd::Checkbox("Override viewmodel", &C_GET(bool, Vars.viewmodell));
            ksd::SliderFloat("X",     &C_GET(float, Vars.viewx), -20.f, 20.f, "%.1f");
            ksd::SliderFloat("Y",     &C_GET(float, Vars.viewy), -20.f, 20.f, "%.1f");
            ksd::SliderFloat("Z",     &C_GET(float, Vars.viewz), -20.f, 20.f, "%.1f");
            ksd::SliderFloat("Model FOV", &C_GET(float, Vars.viewfov), 54.f, 120.f, "%.0f");
        }
        ksd::EndChild();

        if (ksd::BeginChild("Camera", { 4.5f, 5, 4.5f, 5 }))
        {
            ksd::SliderFloat("FOV override", &C_GET(float, Vars.FFov), 60.f, 140.f, "%.0f");
        }
        ksd::EndChild();
    }
}
