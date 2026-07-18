#include "Flags.hpp"
#include "../Menu/Menu.hpp"
#include "../../Dependencies/ImGui/imgui.h"
#include "../../SDK/EntityList.hpp"
#include "../../SDK/Offsets.hpp"
#include "../../SDK/Renderer.hpp"
#include "../../Core/SafeRead.hpp"
#include <windows.h>
#include <string>
#include <cmath>

CFlags g_Flags;

static uint8_t ReadTeam(uintptr_t pawn) {
    uint8_t team = 0;
    SafeRead::ReadByte(pawn + Schemas::C_BaseEntity::m_iTeamNum, team);
    return team;
}

static bool GetPos(uintptr_t pawn, Vector3& out) {
    uintptr_t gsn = 0;
    if (!SafeRead::ReadPtr(pawn + Schemas::C_BaseEntity::m_pGameSceneNode, gsn) || !gsn) return false;
    if (!SafeRead::Read<Vector3>(gsn + Schemas::CGameSceneNode::m_vecAbsOrigin, out)) return false;
    if (out.x == 0.0f && out.y == 0.0f && out.z == 0.0f) return false;
    if (isnan(out.x) || isnan(out.y) || isnan(out.z)) return false;
    return true;
}

void CFlags::RenderFlags() {
    auto* drawList = ImGui::GetBackgroundDrawList();
    if (!drawList) return;

    ViewMatrix vMatrix = Renderer::GetViewMatrix();
    int entityCount = g_EntityList_Logic.GetEntityCount();
    if (entityCount == 0) return;

    static uintptr_t clientBase = 0;
    if (!clientBase) clientBase = reinterpret_cast<uintptr_t>(GetModuleHandleA("client.dll"));
    if (!clientBase || Offsets::dwLocalPlayerPawn == 0) return;

    uintptr_t localPlayer = 0;
    SafeRead::ReadPtr(clientBase + Offsets::dwLocalPlayerPawn, localPlayer);

    uint8_t localTeam = localPlayer ? ReadTeam(localPlayer) : 0;
    float lineH = ImGui::GetTextLineHeight();

    for (int i = 0; i < entityCount && i < 64; i++) {
        EntityData ent = g_EntityList_Logic.GetEntityByIndex(i);
        if (!ent.pawn || !SafeRead::IsValidPtr(ent.pawn)) continue;
        if (ent.pawn == localPlayer) continue;

        int health = 0;
        if (!SafeRead::ReadInt(ent.pawn + Schemas::C_BaseEntity::m_iHealth, health)) continue;
        if (health <= 0 || health > 100) continue;

        uint8_t teamNum = ReadTeam(ent.pawn);
        if (teamNum < 2 || teamNum > 3) continue;
        if (localTeam >= 2 && teamNum == localTeam) continue;

        Vector3 feet = {};
        if (!GetPos(ent.pawn, feet)) continue;

        Vector3 head = { feet.x, feet.y, feet.z + 72.0f };
        Vector2 sFeet, sHead;
        if (!Renderer::WorldToScreen(feet, sFeet, vMatrix)) continue;
        if (!Renderer::WorldToScreen(head, sHead, vMatrix)) continue;

        float h = sFeet.y - sHead.y;
        if (h < 8.0f) continue;

        float w     = h / 2.4f;
        float cx    = (sFeet.x + sHead.x) * 0.5f;
        float xPos  = cx + (w * 0.5f) + 5.0f;
        float yOff  = 0.0f;

        if (f_hp) {
            std::string hpText = std::to_string(health) + "HP";
            drawList->AddText(ImVec2(xPos, sHead.y + yOff), IM_COL32(255, 255, 255, 255), hpText.c_str());
            yOff += lineH + 1.0f;
        }

        if (f_flash) {
            float flash = 0.0f;
            if (SafeRead::ReadFloat(ent.pawn + Schemas::C_CSPlayerPawn::m_flFlashOverlayAlpha, flash) && flash > 0.0f) {
                drawList->AddText(ImVec2(xPos, sHead.y + yOff), IM_COL32(255, 255, 0, 255), "FLASHED");
                yOff += lineH + 1.0f;
            }
        }

        if (f_scope) {
            bool isScoped = false;
            if (SafeRead::ReadBool(ent.pawn + Schemas::C_CSPlayerPawn::m_bIsScoped, isScoped) && isScoped) {
                drawList->AddText(ImVec2(xPos, sHead.y + yOff), IM_COL32(0, 150, 255, 255), "SCOPED");
                yOff += lineH + 1.0f;
            }
        }

        if (f_dist && localPlayer) {
            Vector3 localOrigin = {};
            if (GetPos(localPlayer, localOrigin)) {
                float dx = feet.x - localOrigin.x;
                float dy = feet.y - localOrigin.y;
                float dz = feet.z - localOrigin.z;
                float distance = sqrtf(dx * dx + dy * dy + dz * dz) / 100.0f;
                std::string distText = std::to_string(static_cast<int>(distance)) + "m";
                drawList->AddText(ImVec2(xPos, sHead.y + yOff), IM_COL32(255, 255, 255, 255), distText.c_str());
                yOff += lineH + 1.0f;
            }
        }
    }
}
