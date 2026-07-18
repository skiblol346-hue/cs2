#include "visuals.hpp"
#include "../../framework/shell/shell.hpp"

namespace vars
{
    bool visEnemyEnabled = false;
    int kbVisEnemy = 0;
    bool visBbox = false;
    bool visHealthBar = false;
    int visHealthStyle = 0;
    bool visName = false;
    bool visAvatar = false;
    bool visFlags = false;
    int visFlagItems = 0;
    bool visSkeleton = false;
    bool visWeapon = false;
    int visWeaponStyle = 0;
    bool visGrenadeInv = false;
    bool visAmmo = false;
    bool visNextShot = false;
    bool visDistance = false;
    bool visOffscreen = false;
    int visOffscreenSize = 13;
    int visOffscreenAlpha = 30;
    bool visAimbot = false;
    bool visTracers = false;
    float visTracersTime = 22.f;
    bool visChamsPlayer = false;
    int visChamsMode = 0;
    bool visChamsWall = false;
    bool visGlow = false;
    bool visShadow = false;
    int visShadowMode = 0;
    bool visShadowGlow = false;
    bool visOnShot = false;
    bool visRagdolls = false;

    float visBboxColor[2][4]   = { { 0.f, 0.f, 0.f, 1.f }, { 1.f, 1.f, 1.f, 1.f } };
    float visHealthColor[4]    = { 0.22f, 0.78f, 0.22f, 1.f };
    float visNameColor[4]      = { 1.f, 1.f, 1.f, 1.f };
    float visAvatarColor[4]    = { 1.f, 1.f, 1.f, 0.8f };
    float visFlagsColor[2][4]  = { { 1.f, 1.f, 1.f, 1.f }, { 0.9f, 0.7f, 0.2f, 1.f } };
    float visSkeletonColor[4]  = { 1.f, 1.f, 1.f, 0.6f };
    float visGrenadeColor[4]   = { 0.9f, 0.7f, 0.2f, 1.f };
    float visAmmoColor[4]      = { 0.86f, 0.26f, 0.26f, 1.f };
    float visNextShotColor[4]  = { 0.26f, 0.7f, 0.86f, 1.f };
    float visDistanceColor[4]  = { 1.f, 1.f, 1.f, 0.7f };
    float visOffscreenColor[4] = { 1.f, 0.4f, 0.4f, 1.f };
    float visAimbotColor[4]    = { 0.26f, 0.7f, 0.86f, 1.f };
    float visTracersColor[4]   = { 0.86f, 0.26f, 0.26f, 1.f };

    float visPlayerColor[4]     = { 0.26f, 0.53f, 0.86f, 1.f };
    float visWallPlayerColor[4] = { 0.26f, 0.53f, 0.86f, 0.5f };
    float visGlowColor[4]       = { 0.26f, 0.53f, 0.86f, 0.3f };
    float visShadowColor[4]     = { 0.f, 0.f, 0.f, 0.8f };
    float visShadowGlowColor[4] = { 0.f, 0.f, 0.f, 0.4f };
    float visOnShotColor[4]     = { 0.86f, 0.26f, 0.26f, 1.f };
    float visRagdollsColor[4]   = { 1.f, 0.8f, 0.2f, 1.f };
}

namespace pages
{
    void RenderVisuals() noexcept
    {
        static int visCategory = 0;

        static const char* kHealthBarStyles[] = { "Static", "Dynamic" };
        static const char* kWeaponStyles[]    = { "Icon", "Box", "Both" };
        static const char* kChamModes[]       = { "Solid", "Flat", "Glass", "Wireframe" };
        static const char* kFlagItems[]       = {
            "Kevlar", "Helmet", "Scoped", "Flashed", "Defusing", "Reloading"
        };

        if (ksd::BeginChild("Category", { 0, 0, 9, 0 }))
        {
            constexpr float kCatSlotW = 52.f;
            ksd::TabButton("cat_enemy", &visCategory, 0, 4, true, kCatSlotW);
            ksd::TabButton("cat_team",  &visCategory, 1, 4, true, kCatSlotW);
            ksd::TabButton("cat_world", &visCategory, 2, 4, true, kCatSlotW);
            ksd::TabButton("cat_eye",   &visCategory, 3, 4, true, kCatSlotW);
        }
        ksd::EndChild();

        static const char* kEspLabels[] = {
            "Enemy ESP", "Teammate ESP", "World", "Effects"
        };
        static const char* kModelLabels[] = {
            "Enemy models", "Teammate models", "World objects", "Visual effects"
        };

        if (ksd::BeginChild(kEspLabels[visCategory], { 0, 2, 3, 8 }))
        {
            if (visCategory == 0)
            {
                ksd::CheckboxKeybind("Enabled",
                    &vars::visEnemyEnabled, &vars::kbVisEnemy);

                {
                    const ImVec2 row = ksd::CursorPos();
                    ksd::Checkbox("Bounding box", &vars::visBbox);
                    ksd::ColorButton("##bbox_c0", vars::visBboxColor[0],
                        ImVec2(ksd::PanelRightX() - 37.f, row.y + 2.f), ImVec2(16.f, 10.f));
                    ksd::ColorButton("##bbox_c1", vars::visBboxColor[1],
                        ImVec2(ksd::PanelRightX() - 17.f, row.y + 2.f), ImVec2(16.f, 10.f));
                }

                {
                    const ImVec2 row = ksd::CursorPos();
                    ksd::Checkbox("Health bar", &vars::visHealthBar);
                    ksd::ColorButton("##health_c", vars::visHealthColor,
                        ImVec2(ksd::PanelRightX() - 17.f, row.y + 2.f), ImVec2(16.f, 10.f));
                }
                ksd::SingleSelect("", &vars::visHealthStyle, kHealthBarStyles, 2);

                {
                    const ImVec2 row = ksd::CursorPos();
                    ksd::Checkbox("Name", &vars::visName);
                    ksd::ColorButton("##name_c", vars::visNameColor,
                        ImVec2(ksd::PanelRightX() - 17.f, row.y + 2.f), ImVec2(16.f, 10.f));
                }

                {
                    const ImVec2 row = ksd::CursorPos();
                    ksd::Checkbox("Avatar", &vars::visAvatar);
                    ksd::ColorButton("##avatar_c", vars::visAvatarColor,
                        ImVec2(ksd::PanelRightX() - 17.f, row.y + 2.f), ImVec2(16.f, 10.f));
                }

                {
                    const ImVec2 row = ksd::CursorPos();
                    ksd::Checkbox("Flags", &vars::visFlags);
                    ksd::ColorButton("##flags_c0", vars::visFlagsColor[0],
                        ImVec2(ksd::PanelRightX() - 37.f, row.y + 2.f), ImVec2(16.f, 10.f));
                    ksd::ColorButton("##flags_c1", vars::visFlagsColor[1],
                        ImVec2(ksd::PanelRightX() - 17.f, row.y + 2.f), ImVec2(16.f, 10.f));
                }
                ksd::MultiSelect("", &vars::visFlagItems, kFlagItems, 6);

                {
                    const ImVec2 row = ksd::CursorPos();
                    ksd::Checkbox("Skeleton", &vars::visSkeleton);
                    ksd::ColorButton("##skel_c", vars::visSkeletonColor,
                        ImVec2(ksd::PanelRightX() - 17.f, row.y + 2.f), ImVec2(16.f, 10.f));
                }

                ksd::Checkbox("Weapon", &vars::visWeapon);
                ksd::SingleSelect("", &vars::visWeaponStyle, kWeaponStyles, 3);

                {
                    const ImVec2 row = ksd::CursorPos();
                    ksd::Checkbox("Grenade inventory", &vars::visGrenadeInv);
                    ksd::ColorButton("##gren_c", vars::visGrenadeColor,
                        ImVec2(ksd::PanelRightX() - 17.f, row.y + 2.f), ImVec2(16.f, 10.f));
                }

                {
                    const ImVec2 row = ksd::CursorPos();
                    ksd::Checkbox("Ammo", &vars::visAmmo);
                    ksd::ColorButton("##ammo_c", vars::visAmmoColor,
                        ImVec2(ksd::PanelRightX() - 17.f, row.y + 2.f), ImVec2(16.f, 10.f));
                }

                {
                    const ImVec2 row = ksd::CursorPos();
                    ksd::Checkbox("Next shot", &vars::visNextShot);
                    ksd::ColorButton("##nextshot_c", vars::visNextShotColor,
                        ImVec2(ksd::PanelRightX() - 17.f, row.y + 2.f), ImVec2(16.f, 10.f));
                }

                {
                    const ImVec2 row = ksd::CursorPos();
                    ksd::Checkbox("Distance", &vars::visDistance);
                    ksd::ColorButton("##dist_c", vars::visDistanceColor,
                        ImVec2(ksd::PanelRightX() - 17.f, row.y + 2.f), ImVec2(16.f, 10.f));
                }

                {
                    const ImVec2 row = ksd::CursorPos();
                    ksd::Checkbox("Off-screen arrow", &vars::visOffscreen);
                    ksd::ColorButton("##offscreen_c", vars::visOffscreenColor,
                        ImVec2(ksd::PanelRightX() - 17.f, row.y + 2.f), ImVec2(16.f, 10.f));
                }
                ksd::SliderInt("", &vars::visOffscreenSize, 8, 48, "%dpx");
                ksd::SliderInt("", &vars::visOffscreenAlpha, 0, 100, "%d%%");

                {
                    const ImVec2 row = ksd::CursorPos();
                    ksd::Checkbox("Visualize aimbot", &vars::visAimbot);
                    ksd::ColorButton("##aimbot_c", vars::visAimbotColor,
                        ImVec2(ksd::PanelRightX() - 17.f, row.y + 2.f), ImVec2(16.f, 10.f));
                }

                {
                    const ImVec2 row = ksd::CursorPos();
                    ksd::Checkbox("Bullet tracers", &vars::visTracers);
                    ksd::ColorButton("##tracers_c", vars::visTracersColor,
                        ImVec2(ksd::PanelRightX() - 17.f, row.y + 2.f), ImVec2(16.f, 10.f));
                }
                ksd::SliderFloat("", &vars::visTracersTime, 0.f, 30.f, "%.0fs");
            }
        }
        ksd::EndChild();

        if (ksd::BeginChild(kModelLabels[visCategory], { 6, 2, 3, 8 }))
        {
            if (visCategory == 0 || visCategory == 1)
            {
                {
                    const ImVec2 row = ksd::CursorPos();
                    ksd::Checkbox("Player", &vars::visChamsPlayer);
                    ksd::ColorButton("##champ_c", vars::visPlayerColor,
                        ImVec2(ksd::PanelRightX() - 17.f, row.y + 2.f), ImVec2(16.f, 10.f));
                }
                ksd::SingleSelect("", &vars::visChamsMode, kChamModes, 4);

                {
                    const ImVec2 row = ksd::CursorPos();
                    ksd::Checkbox("Player through walls", &vars::visChamsWall);
                    ksd::ColorButton("##chamw_c", vars::visWallPlayerColor,
                        ImVec2(ksd::PanelRightX() - 17.f, row.y + 2.f), ImVec2(16.f, 10.f));
                }

                {
                    const ImVec2 row = ksd::CursorPos();
                    ksd::Checkbox("Glow", &vars::visGlow);
                    ksd::ColorButton("##glow_c", vars::visGlowColor,
                        ImVec2(ksd::PanelRightX() - 17.f, row.y + 2.f), ImVec2(16.f, 10.f));
                }

                {
                    const ImVec2 row = ksd::CursorPos();
                    ksd::Checkbox("Shadow", &vars::visShadow);
                    ksd::ColorButton("##shadow_c", vars::visShadowColor,
                        ImVec2(ksd::PanelRightX() - 17.f, row.y + 2.f), ImVec2(16.f, 10.f));
                }
                ksd::SingleSelect("", &vars::visShadowMode, kChamModes, 4);

                {
                    const ImVec2 row = ksd::CursorPos();
                    ksd::Checkbox("Shadow glow", &vars::visShadowGlow);
                    ksd::ColorButton("##shadowglow_c", vars::visShadowGlowColor,
                        ImVec2(ksd::PanelRightX() - 17.f, row.y + 2.f), ImVec2(16.f, 10.f));
                }

                {
                    const ImVec2 row = ksd::CursorPos();
                    ksd::Checkbox("On shot", &vars::visOnShot);
                    ksd::ColorButton("##onshot_c", vars::visOnShotColor,
                        ImVec2(ksd::PanelRightX() - 17.f, row.y + 2.f), ImVec2(16.f, 10.f));
                }

                {
                    const ImVec2 row = ksd::CursorPos();
                    ksd::Checkbox("Ragdolls", &vars::visRagdolls);
                    ksd::ColorButton("##ragdolls_c", vars::visRagdollsColor,
                        ImVec2(ksd::PanelRightX() - 17.f, row.y + 2.f), ImVec2(16.f, 10.f));
                }
            }
        }
        ksd::EndChild();
    }
}
