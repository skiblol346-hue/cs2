#include "General.hpp"
#include "../Menu/Menu.hpp"
#include "../../Dependencies/ImGui/imgui.h"
#include "../../SDK/EntityList.hpp"
#include "../../SDK/Offsets.hpp"
#include "../../SDK/Renderer.hpp"
#include "../../Core/SafeRead.hpp"
#include <windows.h>
#include <cmath>
#include <string>

CESP g_ESP;

static uint8_t ReadTeam(uintptr_t pawn) {
    uint8_t team = 0;
    SafeRead::ReadByte(pawn + Schemas::C_BaseEntity::m_iTeamNum, team);
    return team;
}

static bool GetEntityPosition(uintptr_t pawn, Vector3& out) {
    uintptr_t gsn = 0;
    if (!SafeRead::ReadPtr(pawn + Schemas::C_BaseEntity::m_pGameSceneNode, gsn) || !gsn) return false;
    if (!SafeRead::Read<Vector3>(gsn + Schemas::CGameSceneNode::m_vecAbsOrigin, out)) return false;
    if (out.x == 0.0f && out.y == 0.0f && out.z == 0.0f) return false;
    if (isnan(out.x) || isnan(out.y) || isnan(out.z)) return false;
    if (isinf(out.x) || isinf(out.y) || isinf(out.z)) return false;
    return true;
}

static bool GetBonePos(uintptr_t pawn, int boneIndex, Vector3& out) {
    __try {
        uintptr_t gsn = 0;
        if (!SafeRead::ReadPtr(pawn + Schemas::C_BaseEntity::m_pGameSceneNode, gsn) || !gsn)
            return false;

        uintptr_t boneArray = 0;
        if (!SafeRead::ReadPtr(gsn + Schemas::CGameSceneNode::m_modelState + Schemas::CModelState::m_boneArray, boneArray) || !boneArray)
            return false;

        uintptr_t boneAddr = boneArray + static_cast<uintptr_t>(boneIndex) * 32;
        if (!SafeRead::IsValidPtr(boneAddr)) return false;

        float bone[3];
        if (!SafeRead::Read<float[3]>(boneAddr, bone)) return false;

        if (isnan(bone[0]) || isnan(bone[1]) || isnan(bone[2])) return false;
        if (isinf(bone[0]) || isinf(bone[1]) || isinf(bone[2])) return false;
        if (bone[0] == 0.0f && bone[1] == 0.0f && bone[2] == 0.0f) return false;

        out = { bone[0], bone[1], bone[2] };
        return true;
    }
    __except (EXCEPTION_EXECUTE_HANDLER) { return false; }
}

struct BonePair { int from; int to; };

static const BonePair skeletonPairs[] = {
    { 0, 2 },  { 2, 4 },  { 4, 5 },  { 5, 6 },
    { 5, 8 },  { 8, 9 },  { 9, 10 }, { 5, 13 },
    { 13,14 }, { 14,15 }, { 0, 22 }, { 22,23 },
    { 23,24 }, { 0, 25 }, { 25,26 }, { 26,27 },
};

static const int skeletonPairCount = static_cast<int>(sizeof(skeletonPairs) / sizeof(skeletonPairs[0]));

void CESP::RenderGeneral() {
    if (!p_enable) return;

    auto* drawList = ImGui::GetBackgroundDrawList();
    if (!drawList) return;

    int entityCount = g_EntityList_Logic.GetEntityCount();
    if (entityCount == 0) return;

    static uintptr_t clientBase = 0;
    if (!clientBase) clientBase = reinterpret_cast<uintptr_t>(GetModuleHandleA("client.dll"));
    if (!clientBase || Offsets::dwLocalPlayerPawn == 0) return;

    uintptr_t localPawn = 0;
    SafeRead::ReadPtr(clientBase + Offsets::dwLocalPlayerPawn, localPawn);

    uint8_t localTeam = localPawn ? ReadTeam(localPawn) : 0;

    ViewMatrix vMatrix = Renderer::GetViewMatrix();

    for (int i = 0; i < entityCount && i < 64; i++) {
        EntityData ent = g_EntityList_Logic.GetEntityByIndex(i);
        if (!ent.pawn || !SafeRead::IsValidPtr(ent.pawn)) continue;
        if (ent.pawn == localPawn) continue;

        int health = 0;
        if (!SafeRead::ReadInt(ent.pawn + Schemas::C_BaseEntity::m_iHealth, health) || health <= 0 || health > 100) continue;

        uint8_t teamNum = ReadTeam(ent.pawn);
        if (teamNum < 2 || teamNum > 3) continue;
        if (localTeam >= 2 && teamNum == localTeam) continue;

        Vector3 feet = {};
        if (!GetEntityPosition(ent.pawn, feet)) continue;

        Vector3 head = { feet.x, feet.y, feet.z + 72.0f };

        Vector2 sFeet, sHead;
        if (!Renderer::WorldToScreen(feet, sFeet, vMatrix)) continue;
        if (!Renderer::WorldToScreen(head, sHead, vMatrix)) continue;

        float h = sFeet.y - sHead.y;
        if (h < 8.0f) continue;

        float w    = h / 2.4f;
        float cx   = (sFeet.x + sHead.x) * 0.5f;
        float top  = sHead.y;
        float bot  = sFeet.y;
        float left  = cx - w * 0.5f;
        float right = cx + w * 0.5f;

        if (p_name && ent.controller) {
            char playerName[128] = {};
            if (SafeRead::ReadString(ent.controller + Schemas::CCSPlayerController::m_sSanitizedPlayerName, playerName, 128)) {
                ImU32 nameColor = IM_COL32(
                    static_cast<int>(name_col[0] * 255),
                    static_cast<int>(name_col[1] * 255),
                    static_cast<int>(name_col[2] * 255),
                    static_cast<int>(name_col[3] * 255));
                ImVec2 nameSize = ImGui::CalcTextSize(playerName);
                drawList->AddText(ImVec2(cx - nameSize.x * 0.5f, top - nameSize.y - 2.0f), nameColor, playerName);
            }
        }

        if (p_box) {
            ImU32 boxColor = IM_COL32(
                static_cast<int>(box_col[0] * 255),
                static_cast<int>(box_col[1] * 255),
                static_cast<int>(box_col[2] * 255),
                static_cast<int>(box_col[3] * 255));
            drawList->AddRect(ImVec2(left, top), ImVec2(right, bot), boxColor, 0.0f, 0, 1.0f);
        }

        if (p_health) {
            float hpFrac = health / 100.0f;
            float hpH    = h * hpFrac;
            ImU32 hpCol;
            if (health > 50)      hpCol = IM_COL32(0, 255, 0, 255);
            else if (health > 25) hpCol = IM_COL32(255, 255, 0, 255);
            else                  hpCol = IM_COL32(255, 0, 0, 255);
            drawList->AddRectFilled(ImVec2(left - 4, bot - hpH), ImVec2(left - 3, bot), hpCol);
        }

        if (p_armor) {
            int armor = 0;
            if (SafeRead::ReadInt(ent.pawn + Schemas::C_CSPlayerPawn::m_ArmorValue, armor) && armor > 0 && armor <= 100) {
                float armorH = h * (armor / 100.0f);
                drawList->AddRectFilled(ImVec2(right + 3, bot - armorH), ImVec2(right + 4, bot), IM_COL32(0, 128, 255, 255));
            }
        }

        if (p_skeleton) {
            ImU32 skelCol = IM_COL32(
                static_cast<int>(skel_col[0] * 255),
                static_cast<int>(skel_col[1] * 255),
                static_cast<int>(skel_col[2] * 255),
                static_cast<int>(skel_col[3] * 255));

            for (int b = 0; b < skeletonPairCount; b++) {
                Vector3 boneFrom = {}, boneTo = {};
                if (!GetBonePos(ent.pawn, skeletonPairs[b].from, boneFrom)) continue;
                if (!GetBonePos(ent.pawn, skeletonPairs[b].to,   boneTo))   continue;

                Vector2 sFrom, sTo;
                if (!Renderer::WorldToScreen(boneFrom, sFrom, vMatrix)) continue;
                if (!Renderer::WorldToScreen(boneTo,   sTo,   vMatrix)) continue;

                float dx = sFrom.x - sTo.x;
                float dy = sFrom.y - sTo.y;
                float dist = sqrtf(dx * dx + dy * dy);
                if (dist > 300.0f || dist < 1.0f) continue;

                drawList->AddLine(ImVec2(sFrom.x, sFrom.y), ImVec2(sTo.x, sTo.y), skelCol, 1.0f);
            }
        }
    }
}
