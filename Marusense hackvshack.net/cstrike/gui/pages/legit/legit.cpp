#include "legit.hpp"
#include "../../framework/shell/shell.hpp"
#include "../../framework/keybind/keybind.hpp"
#include <Windows.h>
#include "core/variables.h"

namespace vars
{
    int   legitWeaponTab = 0;
    bool  ltEnabled[kWeaponCount]        = {};
    int   ltHitboxes[kWeaponCount]       = {};
    float ltFov[kWeaponCount]            = { 10.f, 10.f, 10.f, 10.f, 10.f, 10.f };
    bool  ltDrawFov[kWeaponCount]        = {};

    bool  ltAutoWall[kWeaponCount]        = {};

    bool  ltTrigger[kWeaponCount]        = {};
    bool  ltTriggerAutoWall[kWeaponCount]= {};
    bool  ltTriggerAlways[kWeaponCount]  = {};
    int   ltTriggerKey[kWeaponCount]     = {};
    int   ltTriggerKeyMode[kWeaponCount] = {};
    int   ltTriggerReaction[kWeaponCount] = {};
    int   ltTriggerDelay[kWeaponCount]   = { 1, 1, 1, 1, 1, 1 };
}

namespace pages
{
    void RenderLegit() noexcept
    {
        if (ksd::BeginChild("Weapon type#Legit", { 0, 0, 9, 1 }))
        {
            int& w = vars::legitWeaponTab;
            ksd::TabButton("usp_silencer", &w, 0, 6, true);
            ksd::TabButton("p90",          &w, 1, 6, true);
            ksd::TabButton("ak47",         &w, 2, 6, true);
            ksd::TabButton("mag7",         &w, 3, 6, true);
            ksd::TabButton("negev",        &w, 4, 6, true);
            ksd::TabButton("ssg08",        &w, 5, 6, true);
        }
        ksd::EndChild();

        const int w = vars::legitWeaponTab;

        static const char* kHitboxes[] = { "Head", "Neck", "Chest", "Pelvis" };
        static const char* kLegitHitboxes[] = { "Head", "Neck", "Chest", "Nearest" };

        if (ksd::BeginChild("Legit Aimbot#Legit", { 0, 1, 4.5f, 9 }))
        {
            ksd::Checkbox("Enabled", &C_GET(bool, Vars.bLegitAimbot));

            {
                const float rowY = ksd::CursorPos().y;
                ksd::Label("Aim key");
                static int aimKey = static_cast<int>(C_GET(unsigned int, Vars.nLegitAimbotKey));
                ksd::DrawKeybindButton("##aimkey",
                    &aimKey, nullptr, rowY,
                    &C_GET(int, Vars.nLegitAimbotKeyMode));
                C_GET(unsigned int, Vars.nLegitAimbotKey) = static_cast<unsigned int>(aimKey);
            }

            ksd::SingleSelect("Hitbox", &C_GET(int, Vars.nLegitHitbox), kLegitHitboxes, 4);

            ksd::SliderFloat("FOV", &C_GET(float, Vars.flLegitFov),
                0.5f, 30.f, "%.1f\xc2\xb0");

            ksd::SliderFloat("Smooth", &C_GET(float, Vars.flLegitSmooth),
                1.f, 50.f, "%.1f");

            ksd::Checkbox("Humanized", &C_GET(bool, Vars.bLegitHumanized));

            ksd::Checkbox("RCS", &C_GET(bool, Vars.bLegitRCS));
            if (C_GET(bool, Vars.bLegitRCS))
            {
                ksd::SliderFloat("RCS amount", &C_GET(float, Vars.flLegitRCSAmount),
                    0.f, 100.f, "%.0f%%");
            }

            ksd::Checkbox("Auto wall", &C_GET(bool, Vars.bLegitAutoWall));

            ksd::Checkbox("Scope only (snipers)", &C_GET(bool, Vars.bLegitScopeOnly));

            ksd::Checkbox("Flash check", &C_GET(bool, Vars.bLegitFlashCheck));

            ksd::SliderFloat("Deadzone", &C_GET(float, Vars.flLegitDeadzone),
                0.f, 2.f, "%.2f\xc2\xb0");

            ksd::SliderFloat("Jitter", &C_GET(float, Vars.flLegitJitter),
                0.f, 1.5f, "%.2f\xc2\xb0");

            ksd::SliderInt("Shot delay", &C_GET(int, Vars.nLegitShotDelay),
                0, 500, "%dms");

            ksd::SliderInt("Kill delay", &C_GET(int, Vars.nLegitKillDelay),
                0, 1000, "%dms");

            ksd::Checkbox("Draw FOV", &C_GET(bool, Vars.bDrawFov));
        }
        ksd::EndChild();

        if (ksd::BeginChild("Seed Triggerbot#Legit", { 4.5f, 1, 4.5f, 9 }))
        {
            ksd::Checkbox("Enabled", &vars::ltTrigger[w]);

            ksd::Checkbox("Auto wall", &vars::ltTriggerAutoWall[w]);

            ksd::Checkbox("Always active", &vars::ltTriggerAlways[w]);

            {
                const float rowY = ksd::CursorPos().y;
                ksd::Label("Trigger key");
                ksd::DrawKeybindButton("##trigkey",
                    &vars::ltTriggerKey[w], nullptr, rowY,
                    &vars::ltTriggerKeyMode[w]);
            }

            ksd::SliderInt("Hitchance", &C_GET(int, Vars.nTriggerHitchance),
                0, 100, C_GET(int, Vars.nTriggerHitchance) == 0 ? "Off" : "%d%%");

            ksd::SliderInt("Min. damage", &C_GET(int, Vars.nTriggerMinDamage),
                0, 126,
                C_GET(int, Vars.nTriggerMinDamage) == 0 ? "Off"
                : (C_GET(int, Vars.nTriggerMinDamage) > 100 ? "HP+%d" : "%d"),
                C_GET(int, Vars.nTriggerMinDamage) > 100 ? 100 : 0);

            ksd::SliderInt("Reaction time", &vars::ltTriggerReaction[w],
                0, 500, "%dms");

            ksd::SliderInt("Delay", &vars::ltTriggerDelay[w],
                0, 500, "%dms");
        }
        ksd::EndChild();
    }
}
