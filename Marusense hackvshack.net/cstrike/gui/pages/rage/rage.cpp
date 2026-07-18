#include "rage.hpp"
#include "../../framework/shell/shell.hpp"
#include "../../framework/styling.hpp"
#include "gui/assets/fonts.hpp"
#include "gui/assets/svg.hpp"
#include "imgui.h"

#include <algorithm>

namespace vars
{
    int rageWeapon = 0;

    // --- per-weapon settings ---
    //                                        Global  AutoSnp SSG08   AWP     R8Rev   Deagle  Pistol  Rifle
    bool rageEnabled[kRageWeaponCount]        = { true,  false,  false,  false,  false,  false,  false,  false };
    int  rageTargetHitboxes[kRageWeaponCount] = { 31,    31,     31,     31,     31,     31,     31,     31    };
    bool rageMultipoint[kRageWeaponCount]     = { true,  true,   true,   true,   true,   true,   true,   true  };
    int  rageMultipointHitboxes[kRageWeaponCount] = { 31, 31,    31,     31,     31,     31,     31,     31    };
    int  rageMultipointScale[kRageWeaponCount]= { 75,   75,     75,     75,     75,     75,     75,     75    };
    int  rageHitchance[kRageWeaponCount]      = { 80,   80,     80,     80,     80,     80,     80,     80    };
    int  rageMindmg[kRageWeaponCount]         = { 100,  100,    100,    100,    100,    100,    100,    100   };
    bool rageAutoWall[kRageWeaponCount]       = { true,  true,   true,   true,   true,   true,   true,   true  };
    bool rageHitchanceOverride[kRageWeaponCount] = {};
    bool rageMindmgOverride[kRageWeaponCount] = { true, true,   true,   true,   true,   true,   true,   true  };
    int  rageMindmgOverrideVal[kRageWeaponCount] = { 14, 14,    14,     14,     14,     14,     14,     14    };
    bool rageForceLethal[kRageWeaponCount]    = {};
    bool rageForceShoot[kRageWeaponCount]     = { true, true,   true,   true,   true,   true,   true,   true  };
    int  rageForceShootHitchance[kRageWeaponCount] = { 45, 45,  45,     45,     45,     45,     45,     45    };
    bool ragePreferBodyAim[kRageWeaponCount]  = {};
    bool rageForceBaim[kRageWeaponCount]      = {};
    bool rageDelayShot[kRageWeaponCount]      = { true, true,   true,   true,   true,   true,   true,   true  };
    int  rageDelayShotTicks[kRageWeaponCount] = { 3,    3,      3,      3,      3,      3,      3,      3     };
    bool rageQuickStop[kRageWeaponCount]      = { true, true,   true,   true,   true,   true,   true,   true  };
    int  rageQuickStopMode[kRageWeaponCount]  = {};
    bool rageAutoscope[kRageWeaponCount]      = {};
    int  rageMaxFov[kRageWeaponCount]         = { 180,  180,    180,    180,    180,    180,    180,    180   };
    int  rageAimSpeed[kRageWeaponCount]       = { 100,  100,    100,    100,    100,    100,    100,    100   };
    int  rageMinVisible[kRageWeaponCount]     = {};

    // --- keybinds (global) ---
    int kbRageEnabled = 0;
    int kbRageMultipoint = 0;
    int kbRageHitchanceOverride = 0;
    int kbRageMindmgOverride = 0;
    int kbRageForceLethal = 0;
    int kbRageForceShoot = 0;
    int kbRageForceBaim = 0;
    int kbRageDelayShot = 0;
    int kbRageQuickStop = 0;

    // --- global settings ---
    int  rageAccuracyBoost = 3;
    bool rageRefineShot = true;
    bool rageAaCorrection = true;
    bool rageAutofire = true;
    bool rageSilentAim = true;
    int  kbRageSilentAim = 0;
    bool rageRemoveRecoil = true;
    bool rageRemoveSpread = true;
    bool rageForceRemoveSpread = false;
    int  kbRageForceRemoveSpread = 0;
    bool rageDoubletap = true;
    int  kbRageDoubletap = 0;
    bool rageQuickPeekAssist = true;
    int  kbRageQuickPeekAssist = 0;
    int  rageQuickPeekMode = 0;
    int  rageQuickPeekDistance = 32;
    bool rageDuckPeekAssist = true;
    int  kbRageDuckPeekAssist = 0;
    bool rageLogMissed = true;
    bool rageLimitAimStep = false;

    // --- anti-aim ---
    bool aaEnabled = true;
    int  kbAaEnabled = 0;
    int  aaPitch = 0;
    int  aaYawBase = 0;
    int  aaYawOffset = 0;
    bool aaJitter = false;
    int  kbAaJitter = 0;
    bool aaSpin = false;
    int  kbAaSpin = 0;
    bool aaFreestanding = false;
    int  kbAaFreestanding = 0;
    int  aaStaticYaw = 0;
    bool aaMouseOverride = true;
    int  kbAaMouseOverride = 0;
    int  aaMouseView = 0;
    int  aaMouseAmount = 100;
    int  aaManualYaw = 90;
    bool aaManualForward = false;
    int  kbAaManualForward = 0;
    int  aaDisablers = 0;

    // --- colors ---
    float rageQpColor[4] = { 0.78f, 0.80f, 0.86f, 1.f };
    float rageLogColor[4] = { 0.86f, 0.20f, 0.20f, 1.f };
    float rageMo1Color[4] = { 0.78f, 0.80f, 0.86f, 1.f };
    float rageMo2Color[4] = { 0.55f, 0.85f, 0.20f, 1.f };
}

namespace pages
{
    void RenderRage() noexcept
    {
        if (ksd::BeginChild("Weapon type", { 0, 0, 3, 1 }))
        {
            static const char* kWeapons[] = {
                "Global", "Auto Sniper", "SSG 08", "AWP", "R8 Revolver", "Desert Eagle", "Pistol", "Rifle"
            };
            static const char* kWeaponSvg[] = { "", "scar20", "ssg08", "awp", "revolver", "deagle", "glock", "ak47" };

            const ImVec2 cursor = ksd::CursorPos();
            const float  rightX = ksd::PanelRightX();
            const float  comboW = (rightX + 3.f) - (cursor.x + 20.f);

            ksd::SingleSelect("", &vars::rageWeapon, kWeapons, 8, comboW, true, kWeaponSvg);

            const int sel = std::clamp(vars::rageWeapon, 0, 7);
            int iconW = 0, iconH = 0;
            const ImTextureID weaponTex = svg::Get(kWeaponSvg[sel], 26, iconW, iconH);
            if (weaponTex)
            {
                const float comboRight = cursor.x + 20.f + comboW;
                const float iconX      = comboRight - iconW - 20.f;
                const float iconY      = cursor.y + 10.f - iconH * 0.5f;
                ImGui::GetWindowDrawList()->AddImage(weaponTex,
                    ImVec2(iconX, iconY), ImVec2(iconX + iconW, iconY + iconH),
                    ImVec2(0.f, 0.f), ImVec2(1.f, 1.f), style::rgba(175, 175, 175));
            }
        }
        ksd::EndChild();

        const int w = std::clamp(vars::rageWeapon, 0, kRageWeaponCount - 1);

        static const char* kHitboxes[] = { "Head", "Chest", "Stomach", "Arms", "Legs" };
        static const char* kQStopModes[] = {
            "Early", "Slow motion", "Move between ticks", "Cycle"
        };

        if (ksd::BeginChild("Aimbot", { 0, 1, 3, 9 }))
        {
            ksd::CheckboxKeybind("Enabled", &vars::rageEnabled[w],
                &vars::kbRageEnabled);

            ksd::MultiSelect("", &vars::rageTargetHitboxes[w],
                kHitboxes, 5);

            ksd::CheckboxKeybind("Multi-point", &vars::rageMultipoint[w],
                &vars::kbRageMultipoint);

            ksd::MultiSelect("", &vars::rageMultipointHitboxes[w],
                kHitboxes, 5);

            if (vars::rageMultipoint[w])
            {
                ksd::SliderInt("", &vars::rageMultipointScale[w],
                    0, 100, "%d%%");
            }

            ksd::SliderInt("Minimum hit chance", &vars::rageHitchance[w],
                0, 100, "%d%%");

            ksd::SliderInt("Minimum damage", &vars::rageMindmg[w],
                0, 126,
                vars::rageMindmg[w] == 0 ? "Auto"
                : (vars::rageMindmg[w] > 100 ? "HP+%d" : "%d"),
                vars::rageMindmg[w] > 100 ? 100 : 0);

            ksd::Checkbox("Auto wall", &vars::rageAutoWall[w]);

            ksd::CheckboxKeybind("Minimum hit chance override",
                &vars::rageHitchanceOverride[w],
                &vars::kbRageHitchanceOverride);

            ksd::CheckboxKeybind("Minimum damage override",
                &vars::rageMindmgOverride[w],
                &vars::kbRageMindmgOverride);

            if (vars::rageMindmgOverride[w])
            {
                ksd::SliderInt("", &vars::rageMindmgOverrideVal[w],
                    0, 100, "%d");
            }

            ksd::CheckboxKeybind("Force lethal shot",
                &vars::rageForceLethal[w],
                &vars::kbRageForceLethal);

            ksd::CheckboxKeybind("Force shoot",
                &vars::rageForceShoot[w],
                &vars::kbRageForceShoot);

            if (vars::rageForceShoot[w])
            {
                ksd::SliderInt("", &vars::rageForceShootHitchance[w],
                    0, 100, "%d%%");
            }

            ksd::Checkbox("Prefer body aim", &vars::ragePreferBodyAim[w]);

            ksd::CheckboxKeybind("Force body aim",
                &vars::rageForceBaim[w],
                &vars::kbRageForceBaim);

            ksd::CheckboxKeybind("Delay shot",
                &vars::rageDelayShot[w],
                &vars::kbRageDelayShot);

            if (vars::rageDelayShot[w])
            {
                ksd::SliderInt("", &vars::rageDelayShotTicks[w],
                    0, 14, "%d");
            }

            ksd::CheckboxKeybind("Quick stop",
                &vars::rageQuickStop[w],
                &vars::kbRageQuickStop);

            if (vars::rageQuickStop[w])
            {
                ksd::SingleSelect("", &vars::rageQuickStopMode[w],
                    kQStopModes, 4);
            }

            ksd::Checkbox("Auto scope", &vars::rageAutoscope[w]);

            ksd::SliderInt("Maximum FOV", &vars::rageMaxFov[w], 0, 180, "%d\xC2\xB0");

            ksd::SliderInt("Aim speed", &vars::rageAimSpeed[w], 0, 100,
                vars::rageAimSpeed[w] >= 100 ? "Instant" : "%d%%");

            ksd::SliderInt("Minimum visible duration", &vars::rageMinVisible[w], 0, 500,
                vars::rageMinVisible[w] == 0 ? "Off" : "%dms");
        }
        ksd::EndChild();

        static const char* kAccBoost[]   = { "Low", "Medium", "High", "Maximum" };
        static const char* kQPeekMode[]  = { "On shot", "On grenade throw" };

        if (ksd::BeginChild("Other#Rage", { 6, 0, 3, 5 }))
        {
            ksd::SingleSelect("Accuracy boost",
                &vars::rageAccuracyBoost, kAccBoost, 4);

            ksd::Checkbox("Refine shot",       &vars::rageRefineShot);
            ksd::Checkbox("Anti-aim correction", &vars::rageAaCorrection);
            ksd::Checkbox("Automatic fire",    &vars::rageAutofire);

            ksd::CheckboxKeybind("Silent aim",
                &vars::rageSilentAim,
                &vars::kbRageSilentAim);

            ksd::Checkbox("Remove recoil", &vars::rageRemoveRecoil);
            ksd::Checkbox("Remove spread", &vars::rageRemoveSpread);

            const bool spreadEnabled = vars::rageRemoveSpread;
            bool& forceSpread = vars::rageForceRemoveSpread;
            if (!spreadEnabled) forceSpread = false;
            ksd::CheckboxKeybind("Force remove spread",
                &forceSpread,
                &vars::kbRageForceRemoveSpread);

            ksd::CheckboxKeybind("Double tap",
                &vars::rageDoubletap,
                &vars::kbRageDoubletap);

            ksd::CheckboxKeybind("Quick peek assist",
                &vars::rageQuickPeekAssist,
                &vars::kbRageQuickPeekAssist);

            if (vars::rageQuickPeekAssist)
            {
                const ImVec2 row    = ksd::CursorPos();
                const float  rightX = ksd::PanelRightX();
                const float  modeW  = rightX - row.x - 41.f;

                ksd::SingleSelect("", &vars::rageQuickPeekMode,
                    kQPeekMode, 2, modeW);

                ksd::ColorButton("##qpeek_color", vars::rageQpColor,
                    ImVec2(rightX - 17.f, row.y + 6.f), ImVec2(16.f, 10.f));

                ksd::SliderInt("Maximum peek distance",
                    &vars::rageQuickPeekDistance, 0, 500, "%d");
            }

            ksd::CheckboxKeybind("Duck peek assist",
                &vars::rageDuckPeekAssist,
                &vars::kbRageDuckPeekAssist);

            {
                const ImVec2 row = ksd::CursorPos();
                ksd::Checkbox("Log missed shots", &vars::rageLogMissed);
                ksd::ColorButton("##logmissed_color", vars::rageLogColor,
                    ImVec2(ksd::PanelRightX() - 17.f, row.y + 2.f), ImVec2(16.f, 10.f));
            }

            ksd::Checkbox("Limit aim step", &vars::rageLimitAimStep);
        }
        ksd::EndChild();

        static const char* kPitch[]     = { "Down", "Up", "Off" };
        static const char* kYawBase[]   = { "At targets", "Local", "Backward", "Side" };
        static const char* kStaticYaw[] = { "Off", "Left", "Right", "Backward", "Side" };
        static const char* kMouseView[] = { "Local view", "At targets", "Off" };
        static const char* kDisablers[] = { "Anti-backstab", "On use", "On peek", "Off" };

        if (ksd::BeginChild("Anti-aimbot angles", { 6, 7, 3, 3 }))
        {
            ksd::CheckboxKeybind("Enabled",
                &vars::aaEnabled,
                &vars::kbAaEnabled);

            ksd::SingleSelect("Pitch", &vars::aaPitch, kPitch, 3);
            ksd::SingleSelect("Yaw base", &vars::aaYawBase, kYawBase, 4);

            ksd::SliderInt("", &vars::aaYawOffset,
                -180, 180, "%d\xC2\xB0");

            ksd::CheckboxKeybind("Jitter",
                &vars::aaJitter,
                &vars::kbAaJitter);

            ksd::CheckboxKeybind("Spin",
                &vars::aaSpin,
                &vars::kbAaSpin);

            ksd::CheckboxKeybind("Freestanding",
                &vars::aaFreestanding,
                &vars::kbAaFreestanding);

            ksd::SingleSelect("Static yaw", &vars::aaStaticYaw, kStaticYaw, 5);

            {
                const ImVec2 row    = ksd::CursorPos();
                const float  rightX = ksd::PanelRightX();
                ksd::CheckboxKeybind("Mouse override",
                    &vars::aaMouseOverride,
                    &vars::kbAaMouseOverride);
                ksd::ColorButton("##mo_col1", vars::rageMo1Color, ImVec2(rightX - 37.f, row.y + 2.f), ImVec2(16.f, 10.f));
                ksd::ColorButton("##mo_col2", vars::rageMo2Color, ImVec2(rightX - 17.f, row.y + 2.f), ImVec2(16.f, 10.f));
            }

            if (vars::aaMouseOverride)
            {
                ksd::SingleSelect("View", &vars::aaMouseView, kMouseView, 3);
                ksd::SliderInt("", &vars::aaMouseAmount, 0, 100, "%d%%");
            }

            ksd::SliderInt("Manual yaw (left / right)",
                &vars::aaManualYaw, -90, 90, "%d\xC2\xB0");

            ksd::CheckboxKeybind("Manual yaw (forward)",
                &vars::aaManualForward,
                &vars::kbAaManualForward);

            ksd::SingleSelect("Disablers", &vars::aaDisablers, kDisablers, 4);
        }
        ksd::EndChild();
    }
}
