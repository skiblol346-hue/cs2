#include "legit.hpp"
#include "../../framework/shell/shell.hpp"

namespace vars
{
    int legitWeaponTab = 0;
    bool ltEnabled = false;
    int kbLtEnabled = 0;
    int ltAimHitbox = 0;
    float ltFov = 10.f;
    float ltSpeed = 0.65f;
    float ltSpeedAtk = 0.65f;
    int ltSpeedScale = 100;
    int ltReaction = 100;
    int ltLocktime = 4000;
    bool ltQuickStop = false;
    int ltAimCond = 0;
    bool ltTrigger = false;
    int kbLtTrigger = 0;
    int ltTriggerHitbox = 0;
    int ltTriggerHitchance = 75;
    int ltTriggerMindmg = 1;
    int ltTriggerDelay = 0;
    int ltTriggerReaction = 0;
    bool ltBurstFire = false;
    int ltShootCond = 0;
    int ltAccuracyBoost = 0;
}

namespace pages
{
    void RenderLegit() noexcept
    {
        if (ksd::BeginChild("Weapon type#Legit", { 0, 0, 9, 0 }))
        {
            int& legitTab = vars::legitWeaponTab;
            ksd::TabButton("usp_silencer", &legitTab, 0, 6, true);
            ksd::TabButton("p90",          &legitTab, 1, 6, true);
            ksd::TabButton("ak47",         &legitTab, 2, 6, true);
            ksd::TabButton("mag7",         &legitTab, 3, 6, true);
            ksd::TabButton("negev",        &legitTab, 4, 6, true);
            ksd::TabButton("ssg08",        &legitTab, 5, 6, true);
        }
        ksd::EndChild();

        static const char* kHitboxes[]   = { "Head", "Chest", "Stomach", "Arms", "Legs", "Feet" };
        static const char* kAimConds[]   = { "\xe2\x80\x93", "Stand still", "Walk", "In attack" };
        static const char* kShootConds[] = { "\xe2\x80\x93", "Stand still", "Walk", "In attack" };
        static const char* kAccBoost[]   = { "Off", "Low", "Medium", "High", "Maximum" };

        if (ksd::BeginChild("Aimbot#Legit", { 0, 2, 3, 8 }))
        {
            ksd::CheckboxKeybind("Enabled", &vars::ltEnabled, &vars::kbLtEnabled);

            ksd::SingleSelect("", &vars::ltAimHitbox, kHitboxes, 6);

            ksd::SliderFloat("Maximum FOV", &vars::ltFov,
                0.f, 30.f, "%.1f\xc2\xb0");

            ksd::SliderFloat("Speed", &vars::ltSpeed, 0.f, 1.f, "%.2f");

            ksd::SliderFloat("Speed (in attack)", &vars::ltSpeedAtk,
                0.f, 1.f, "%.2f");

            ksd::SliderInt("Speed scale - FOV", &vars::ltSpeedScale, 0, 100, "%d%%");

            ksd::SliderInt("Reaction time", &vars::ltReaction, 0, 500, "%dms");

            ksd::SliderInt("Maximum lock-on time", &vars::ltLocktime,
                0, 4000,
                vars::ltLocktime >= 4000 ? "\xe2\x88\x9e" : "%dms");

            ksd::Checkbox("Quick stop", &vars::ltQuickStop);

            ksd::SingleSelect("Aim conditions", &vars::ltAimCond, kAimConds, 4);
        }
        ksd::EndChild();

        if (ksd::BeginChild("Triggerbot", { 6, 2, 3, 5 }))
        {
            ksd::CheckboxKeybind("Enabled", &vars::ltTrigger, &vars::kbLtTrigger);

            ksd::SingleSelect("", &vars::ltTriggerHitbox, kHitboxes, 6);

            ksd::SliderInt("Minimum hit chance", &vars::ltTriggerHitchance,
                0, 100, "%d%%");

            ksd::SliderInt("Minimum damage", &vars::ltTriggerMindmg, 1, 100, "%d");

            ksd::SliderInt("Delay", &vars::ltTriggerDelay, 0, 500, "%dms");

            ksd::SliderInt("Reaction time", &vars::ltTriggerReaction, 0, 500, "%dms");

            ksd::Checkbox("Burst fire", &vars::ltBurstFire);

            ksd::SingleSelect("Shoot conditions", &vars::ltShootCond, kShootConds, 4);
        }
        ksd::EndChild();

        if (ksd::BeginChild("Other#Legit", { 6, 9, 3, 1 }))
        {
            ksd::SingleSelect("Accuracy boost", &vars::ltAccuracyBoost, kAccBoost, 5);
        }
        ksd::EndChild();
    }
}
