#include "visuals.hpp"
#include "../../framework/shell/shell.hpp"
#include "../../framework/styling.hpp"
#include <Windows.h>
#include "core/variables.h"

namespace pages
{
    void RenderVisuals() noexcept
    {
        static const char* kChamMats[] = {
            "White", "Illuminate", "Latex", "Ghost"
        };

        if (ksd::BeginChild("Players ESP", { 0, 0, 3, 10 }))
        {
            ksd::Checkbox("Bounding box",   &C_GET(bool, Vars.bBox));
            ksd::Checkbox("Box outline",    &C_GET(bool, Vars.bBoxOutline));
            ksd::Checkbox("Skeleton",       &C_GET(bool, Vars.bSkeleton));
            ksd::Checkbox("Health bar",     &C_GET(bool, Vars.bHealthBar));
            ksd::Checkbox("Health text",    &C_GET(bool, Vars.bHealthText));
            ksd::Checkbox("Name",           &C_GET(bool, Vars.bName));
            ksd::Checkbox("Weapon",         &C_GET(bool, Vars.bWeapon));
            ksd::Checkbox("Snaplines",      &C_GET(bool, Vars.bSnaplines));
            ksd::Checkbox("Head dot",       &C_GET(bool, Vars.bHeadDot));
            ksd::Checkbox("Ammo bar",       &C_GET(bool, Vars.bAmmoBar));
            ksd::Checkbox("Armor bar",      &C_GET(bool, Vars.bArmorBar));
            ksd::Checkbox("Flags",          &C_GET(bool, Vars.bFlags));
            if (C_GET(bool, Vars.bFlags))
            {
                ksd::Checkbox("  Armor",    &C_GET(bool, Vars.bFlagArmor));
                ksd::Checkbox("  Scoped",   &C_GET(bool, Vars.bFlagScoped));
                ksd::Checkbox("  Defusing", &C_GET(bool, Vars.bFlagDefusing));
                ksd::Checkbox("  Kit",      &C_GET(bool, Vars.bFlagKit));
                ksd::Checkbox("  Flashed",  &C_GET(bool, Vars.bFlagFlashed));
            }
            ksd::Checkbox("Distance",       &C_GET(bool, Vars.bDistance));
        }
        ksd::EndChild();

        if (ksd::BeginChild("Chams", { 3, 0, 3, 5 }))
        {
            ksd::Checkbox("Player chams",   &C_GET(bool, Vars.bVisualChams));
            ksd::Checkbox("Ignore-Z",       &C_GET(bool, Vars.bVisualChamsIgnoreZ));
            ksd::SingleSelect("Material",
                &C_GET(int, Vars.nVisualChamMaterial), kChamMats, 4);
            ksd::Checkbox("Weapon chams",   &C_GET(bool, Vars.bVisualChamsWeapon));
            ksd::Checkbox("Arms chams",     &C_GET(bool, Vars.bVisualChamsArms));
            ksd::SingleSelect("Weapon material",
                &C_GET(int, Vars.nVisualChamMaterialWeapon), kChamMats, 4);
        }
        ksd::EndChild();

        if (ksd::BeginChild("World visuals", { 6, 0, 3, 5 }))
        {
            ksd::Checkbox("Smoke radius",   &C_GET(bool, Vars.bSmokeRadius));
            ksd::Checkbox("Inferno radius", &C_GET(bool, Vars.bInfernoRadius));
            ksd::Checkbox("Dropped weapons", &C_GET(bool, Vars.bDroppedWeapons));
            ksd::Checkbox("Bomb timer",     &C_GET(bool, Vars.bBombTimer));
            ksd::Checkbox("Bomb ESP",       &C_GET(bool, Vars.bBombESP));
            ksd::Checkbox("Spectator list", &C_GET(bool, Vars.bSpectatorList));
            ksd::Checkbox("Hit marker",     &C_GET(bool, Vars.bHitMarker));
            ksd::Checkbox("Damage indicator", &C_GET(bool, Vars.bDamageIndicator));
        }
        ksd::EndChild();

    }
}
