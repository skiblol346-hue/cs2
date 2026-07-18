#include "misc.hpp"
#include "../../framework/shell/shell.hpp"

namespace vars
{
    bool miscAfkKick = false;
    bool miscUnlockLoadout = false;
    bool miscPanorama = false;
    bool miscKnifebot = false;
    int kbKnifebot = 0;
    bool miscKnifebotStab = false;
    bool miscKnifebotAuto = false;
    int kbKnifebotAuto = 0;
    bool miscGeco = false;
    bool miscQuickSwitch = false;
    bool miscQuickPlant = false;
    bool miscLastSecObj = false;
    bool miscSuperToss = false;
    bool miscAutoNade = false;
    int miscNadeDelay = 42;
    bool miscViewmodelSide = false;
    int miscViewmodelSideOpt = 0;
    int miscViewmodelScale = 50;
    bool miscKillSound = false;
    bool miscHeadshotSound = false;
    bool miscAutoBuy = false;
    int miscAutoBuyWpn = 0;
    bool miscClanTag = false;
    bool miscLogDamage = false;
    int kbLogDamage = 0;
    bool miscRemoveRoles = false;
    bool miscBhop = false;
    int kbBhop = 0;
    bool miscAirStrafe = false;
    int miscStrafeCorrection = 100;
    bool miscSubTrick = false;
    bool miscAvoidObstacles = false;
    bool miscJumpBug = false;
    int kbJumpBug = 0;
    bool miscNoFallDmg = false;
    bool miscNoLandInaccuracy = false;
    bool miscAirDuck = false;
    bool miscJumpBoost = false;
    bool miscJumpAtEdge = false;
    int miscJumpEdgeDist = 2;
    int miscJumpEdgeAngle = 3;
    bool miscStopAtEdge = false;
    float miscStopEdgeDist = 44.71f;
    int miscStopEdgePad = 3;
    bool miscEasyStrafe = false;
    bool miscFastWalk = false;
    bool miscFastLadder = false;
    bool miscStandaloneQs = false;
    bool miscSlowMotion = false;
    int miscSlowMotionSpeed = 25;
    bool miscAutoAccept = false;
    bool miscOverrideRegion = false;
    bool miscRevealRanks = false;
}

namespace pages
{
    void RenderMisc() noexcept
    {
        static const char* kViewSides[]   = { "Default", "Left", "Right" };
        static const char* kAutoBuyWpns[] = {
            "SSG 08", "AWP", "M4A4", "AK-47", "Desert Eagle", "UMP-45"
        };

        if (ksd::BeginChild("Miscellaneous", { 0, 0, 3, 10 }))
        {
            ksd::Checkbox("Prevent AFK kick",      &vars::miscAfkKick);
            ksd::Checkbox("Unlock loadout",         &vars::miscUnlockLoadout);
            ksd::Checkbox("Panorama UI extensions", &vars::miscPanorama);

            ksd::CheckboxKeybind("Knifebot",
                &vars::miscKnifebot, &vars::kbKnifebot);

            if (vars::miscKnifebot)
            {
                ksd::Checkbox("Full stab", &vars::miscKnifebotStab);
                ksd::CheckboxKeybind("Automatic weapons",
                    &vars::miscKnifebotAuto, &vars::kbKnifebotAuto);
                ksd::Checkbox("Geco", &vars::miscGeco);
            }

            ksd::Checkbox("Quick switch",          &vars::miscQuickSwitch);
            ksd::Checkbox("Quick plant",           &vars::miscQuickPlant);
            ksd::Checkbox("Last second objective", &vars::miscLastSecObj);
            ksd::Checkbox("Super toss",            &vars::miscSuperToss);

            ksd::Checkbox("Automatic grenade release", &vars::miscAutoNade);
            ksd::SliderInt("", &vars::miscNadeDelay, 0, 500, "%d");

            ksd::Checkbox("Switch knife viewmodel side", &vars::miscViewmodelSide);
            if (vars::miscViewmodelSide)
            {
                ksd::SingleSelect("", &vars::miscViewmodelSideOpt, kViewSides, 3);
                ksd::SliderInt("", &vars::miscViewmodelScale, 0, 100, "%d%%");
            }

            ksd::Checkbox("Kill sound",     &vars::miscKillSound);
            ksd::Checkbox("Headshot sound", &vars::miscHeadshotSound);

            ksd::Checkbox("Auto buy", &vars::miscAutoBuy);
            if (vars::miscAutoBuy)
                ksd::SingleSelect("", &vars::miscAutoBuyWpn, kAutoBuyWpns, 6);

            ksd::Checkbox("Clan tag spammer", &vars::miscClanTag);
            if (vars::miscClanTag)
            {
                static char clanTag[32] = {};
                ksd::InputText("##clantag", clanTag, sizeof(clanTag));
            }

            ksd::CheckboxKeybind("Log damage dealt",
                &vars::miscLogDamage, &vars::kbLogDamage);

            ksd::Checkbox("Remove roles", &vars::miscRemoveRoles);
        }
        ksd::EndChild();

        if (ksd::BeginChild("Movement", { 6, 0, 3, 7 }))
        {
            ksd::CheckboxKeybind("Bunny hop",
                &vars::miscBhop, &vars::kbBhop);

            ksd::Checkbox("Air strafe", &vars::miscAirStrafe);
            ksd::Label("Movement keys");
            ksd::SliderInt("", &vars::miscStrafeCorrection, 0, 100, "%d%%");
            ksd::Checkbox("Sub-trick air strafe", &vars::miscSubTrick);

            ksd::Checkbox("Avoid obstacles",  &vars::miscAvoidObstacles);
            ksd::CheckboxKeybind("Jump bug",
                &vars::miscJumpBug, &vars::kbJumpBug);
            ksd::Checkbox("No fall damage",     &vars::miscNoFallDmg);
            ksd::Checkbox("No land inaccuracy", &vars::miscNoLandInaccuracy);
            ksd::Checkbox("Air duck",           &vars::miscAirDuck);
            ksd::Checkbox("Jump boost",         &vars::miscJumpBoost);

            ksd::Checkbox("Jump at edge", &vars::miscJumpAtEdge);
            ksd::SliderInt("", &vars::miscJumpEdgeDist,  0, 20, "%d");
            ksd::SliderInt("", &vars::miscJumpEdgeAngle, 0, 10, "%d");

            ksd::Checkbox("Stop at edge", &vars::miscStopAtEdge);
            ksd::SliderFloat("", &vars::miscStopEdgeDist, 0.f, 100.f, "%.2f");
            ksd::SliderInt("",  &vars::miscStopEdgePad, 0, 10, "%d");

            ksd::Checkbox("Easy strafe",           &vars::miscEasyStrafe);
            ksd::Checkbox("Fast walk",             &vars::miscFastWalk);
            ksd::Checkbox("Fast ladder",           &vars::miscFastLadder);
            ksd::Checkbox("Standalone quick stop", &vars::miscStandaloneQs);

            ksd::Checkbox("Slow motion", &vars::miscSlowMotion);
            ksd::SliderInt("", &vars::miscSlowMotionSpeed, 0, 100, "%d%%");
        }
        ksd::EndChild();

        if (ksd::BeginChild("Matchmaking", { 6, 9, 3, 1 }))
        {
            ksd::Checkbox("Auto-accept matchmaking",     &vars::miscAutoAccept);
            ksd::Checkbox("Override matchmaking region", &vars::miscOverrideRegion);
            ksd::Checkbox("Reveal matchmaking ranks",    &vars::miscRevealRanks);
        }
        ksd::EndChild();
    }
}
