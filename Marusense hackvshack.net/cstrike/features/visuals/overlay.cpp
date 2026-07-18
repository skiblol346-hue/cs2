// used: [stl] vector
#include <vector>
// used: [stl] sort
#include <algorithm>

#include "overlay.h"

// used: cheat variables
#include "../../core/variables.h"

// used: entity
#include "../../sdk/entity.h"
#include "../../sdk/interfaces/cgameentitysystem.h"

#include "../../sdk/interfaces/cgametracemanager.h"


#include "../../core/sdk.h"


#include "../../utilities/log.h"

#include "../../utilities/inputsystem.h"

#include "../../utilities/draw.h"


#include "../../core/menu.h"
#define _USE_MATH_DEFINES
#include <cmath>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

ImColor GetRainbowColor(float offset = 0.0f) {
    float time = ImGui::GetTime() * 0.5f;
    float hue = fmod(time + offset, 1.0f);
    return ImColor::HSV(hue, 1.0f, 1.0f);
}




bool GetEntityBoundingBox(C_CSPlayerPawn* pEntity, ImVec4* pVecOut)
{
    CCollisionProperty* pCollision = pEntity->GetCollision();
    if (pCollision == nullptr)
        return false;

    CGameSceneNode* pGameSceneNode = pEntity->GetGameSceneNode();
    if (pGameSceneNode == nullptr)
        return false;

    CTransform nodeToWorldTransform = pGameSceneNode->GetNodeToWorld();
    const Matrix3x4_t matTransform = nodeToWorldTransform.quatOrientation.ToMatrix(nodeToWorldTransform.vecPosition);

    const Vector_t vecMins = pCollision->GetMins();
    const Vector_t vecMaxs = pCollision->GetMaxs();

    pVecOut->x = pVecOut->y = std::numeric_limits<float>::max();
    pVecOut->z = pVecOut->w = -std::numeric_limits<float>::max();

    for (int i = 0; i < 8; ++i)
    {
        const Vector_t vecPoint{
            i & 1 ? vecMaxs.x : vecMins.x,
            i & 2 ? vecMaxs.y : vecMins.y,
            i & 4 ? vecMaxs.z : vecMins.z
        };
        ImVec2 vecScreen;
        if (!D::WorldToScreen(vecPoint.Transform(matTransform), &vecScreen))
            return false;

        pVecOut->x = MATH::Min(pVecOut->x, vecScreen.x);
        pVecOut->y = MATH::Min(pVecOut->y, vecScreen.y);
        pVecOut->z = MATH::Max(pVecOut->z, vecScreen.x);
        pVecOut->w = MATH::Max(pVecOut->w, vecScreen.y);
    }

    return true;
}

const char* GetWeaponNameByIndex(int id)
{
    switch (id)
    {
     
    case WEAPON_DESERT_EAGLE: return "Deagle";
    case WEAPON_DUAL_BERETTAS: return "Dual Berettas";
    case WEAPON_FIVE_SEVEN: return "Five-Seven";
    case WEAPON_GLOCK_18: return "Glock-18";
    case WEAPON_TEC_9: return "Tec-9";
    case WEAPON_P2000: return "P2000";
    case WEAPON_P250: return "P250";
    case WEAPON_USP_S: return "USP-S";
    case WEAPON_CZ75_AUTO: return "CZ75-Auto";
    case WEAPON_R8_REVOLVER: return "R8 Revolver";

    case WEAPON_AK_47: return "AK-47";
    case WEAPON_AUG: return "AUG";
    case WEAPON_AWP: return "AWP";
    case WEAPON_FAMAS: return "FAMAS";
    case WEAPON_G3SG1: return "G3SG1";
    case WEAPON_GALIL_AR: return "Galil AR";
    case WEAPON_M4A4: return "M4A4";
    case WEAPON_M4A1_S: return "M4A1-S";
    case WEAPON_SG_553: return "SG 553";
    case WEAPON_SCAR_20: return "SCAR-20";
    case WEAPON_SSG_08: return "SSG 08";

      
    case WEAPON_MAC_10: return "MAC-10";
    case WEAPON_MP5_SD: return "MP5-SD";
    case WEAPON_MP7: return "MP7";
    case WEAPON_MP9: return "MP9";
    case WEAPON_P90: return "P90";
    case WEAPON_UMP_45: return "UMP-45";
    case WEAPON_PP_BIZON: return "PP-Bizon";

    case WEAPON_XM1014: return "XM1014";
    case WEAPON_MAG_7: return "MAG-7";
    case WEAPON_NOVA: return "Nova";
    case WEAPON_SAWED_OFF: return "Sawed-Off";

        
    case WEAPON_M249: return "M249";
    case WEAPON_NEGEV: return "Negev";

    case WEAPON_FLASHBANG: return "Flashbang";
    case WEAPON_HIGH_EXPLOSIVE_GRENADE: return "HE Grenade";
    case WEAPON_SMOKE_GRENADE: return "Smoke";
    case WEAPON_MOLOTOV: return "Molotov";
    case WEAPON_DECOY_GRENADE: return "Decoy";
    case WEAPON_INCENDIARY_GRENADE: return "Incendiary";
    case WEAPON_SNOWBALL: return "Snowball";

    
    case WEAPON_C4_EXPLOSIVE: return "C4";
    case WEAPON_DEFUSE_KIT: return "Defuse Kit";
    case WEAPON_MEDI_SHOT: return "Medi-Shot";


    case WEAPON_ZEUS_X27: return "Zeus";


    case WEAPON_KNIFE0:
    case WEAPON_KNIFE1:
    case WEAPON_KNIFE2:
    case WEAPON_KNIFE3: return "Knife";

    case WEAPON_BAYONET: return "Bayonet";
    case WEAPON_CLASSIC_KNIFE: return "Classic Knife";
    case WEAPON_FLIP_KNIFE: return "Flip Knife";
    case WEAPON_GUT_KNIFE: return "Gut Knife";
    case WEAPON_KARAMBIT: return "Karambit";
    case WEAPON_M9_BAYONET: return "M9 Bayonet";
    case WEAPON_HUNTSMAN_KNIFE: return "Huntsman Knife";
    case WEAPON_FALCHION_KNIFE: return "Falchion Knife";
    case WEAPON_BOWIE_KNIFE: return "Bowie Knife";
    case WEAPON_BUTTERFLY_KNIFE: return "Butterfly Knife";
    case WEAPON_SHADOW_DAGGERS: return "Shadow Daggers";
    case WEAPON_PARACORD_KNIFE: return "Paracord Knife";
    case WEAPON_SURVIVAL_KNIFE: return "Survival Knife";
    case WEAPON_URSUS_KNIFE: return "Ursus Knife";
    case WEAPON_NAVAJA_KNIFE: return "Navaja Knife";
    case WEAPON_NOMAD_KNIFE: return "Nomad Knife";
    case WEAPON_STILETTO_KNIFE: return "Stiletto Knife";
    case WEAPON_TALON_KNIFE: return "Talon Knife";
    case WEAPON_SKELETON_KNIFE: return "Skeleton Knife";

    default:
        return "Unknown";
    }
}

inline Vector_t GetBonePos(C_CSPlayerPawn* pawn, BoneID boneID)
{
    if (!pawn)
        return {};

    auto skeleton = reinterpret_cast<CSkeletonInstance*>(pawn->GetGameSceneNode());
    if (!skeleton)
        return {};

    auto modelState = &skeleton->GetModelState();
    if (!modelState)
        return {};

    uintptr_t boneArray = *reinterpret_cast<uintptr_t*>(
        reinterpret_cast<uintptr_t>(modelState) + 0x80
        );

    if (!boneArray)
        return {};

    return *reinterpret_cast<Vector_t*>(boneArray + static_cast<int>(boneID) * 0x20);
}



inline ImColor LerpColor(const ImColor& color1, const ImColor& color2, float t) {
    return ImColor(
        color1.Value.x + (color2.Value.x - color1.Value.x) * t,
        color1.Value.y + (color2.Value.y - color1.Value.y) * t,
        color1.Value.z + (color2.Value.z - color1.Value.z) * t,
        color1.Value.w + (color2.Value.w - color1.Value.w) * t
    );
}

void DrawSkeleton(C_CSPlayerPawn* pPawn) {
    if (!pPawn || !C_GET(bool, Vars.bSkeleton))
        return;

    ImColor color1, color2;


        Color_t col1 = C_GET(Color_t, Vars.colSkeleton1);

        color1 = ImColor((float)col1.r, (float)col1.g, (float)col1.b, (float)col1.a);

    

  
    int totalConnections = connections.size();
    int currentIndex = 0;

    for (const auto& connection : connections) {
        Vector_t bonePos1 = GetBonePos(pPawn, connection.bone1);
        Vector_t bonePos2 = GetBonePos(pPawn, connection.bone2);

        if (bonePos1.IsZero() || bonePos2.IsZero()) {
            currentIndex++;
            continue;
        }

        ImVec2 screenPos1, screenPos2;
        if (!D::WorldToScreen(bonePos1, &screenPos1)) {
            currentIndex++;
            continue;
        }
        if (!D::WorldToScreen(bonePos2, &screenPos2)) {
            currentIndex++;
            continue;
        }

        ImColor drawColor;


        bool isLegs = (connection.bone1 == BoneID::Pelvis ||
            connection.bone1 == BoneID::LeftHip ||
            connection.bone1 == BoneID::RightHip ||
            connection.bone2 == BoneID::LeftFoot ||
            connection.bone2 == BoneID::RightFoot);


            drawColor =  color1;
        

        ImGui::GetBackgroundDrawList()->AddLine(screenPos1, screenPos2, drawColor, 2.0f);
        currentIndex++;
    }
}
void DrawCornerBox(ImDrawList* draw, const ImVec2& min, const ImVec2& max, const ImColor& color, float thickness = 1.5f, float cornerLength = 8.0f)
{

    draw->AddLine(ImVec2(min.x, min.y + cornerLength), min, color, thickness);
    draw->AddLine(ImVec2(min.x + cornerLength, min.y), min, color, thickness);


    draw->AddLine(ImVec2(max.x - cornerLength, min.y), max, color, thickness);
    draw->AddLine(ImVec2(max.x, min.y + cornerLength), ImVec2(max.x, min.y), color, thickness);


    draw->AddLine(ImVec2(min.x, max.y - cornerLength), ImVec2(min.x, max.y), color, thickness);
    draw->AddLine(ImVec2(min.x + cornerLength, max.y), ImVec2(min.x, max.y), color, thickness);


    draw->AddLine(ImVec2(max.x - cornerLength, max.y), ImVec2(max.x, max.y), color, thickness);
    draw->AddLine(ImVec2(max.x, max.y - cornerLength), ImVec2(max.x, max.y), color, thickness);
}
static std::unordered_map<int, float> smokeAlpha;

void F::VISUALS::ESP::Render()
{
    if (!SDK::LocalPawn)
        return;

    const int nHighestIndex = I::GameResourceService->pGameEntitySystem->GetHighestEntityIndex();

    for (int nIndex = 1; nIndex <= nHighestIndex; nIndex++)
    {
        C_BaseEntity* pEntity = I::GameResourceService->pGameEntitySystem->Get(nIndex);
        if (pEntity == nullptr)
            continue;

        CSchemaClassInfo* pClassInfo = nullptr;
        pEntity->GetSchemaClassInfo(&pClassInfo);

        if (pClassInfo == nullptr)
            continue;
        static std::unordered_map<int, float> infernoAlpha;

        if (FNV1A::Hash(pClassInfo->Name) == FNV1A::HashConst("C_Inferno"))
        {
            if (!C_GET(bool, Vars.bInfernoRadius))
            {
                infernoAlpha.clear();
                continue;
            }

            C_Inferno* pInferno = reinterpret_cast<C_Inferno*>(pEntity);
            if (!pInferno)
                continue;

            ImDrawList* draw = ImGui::GetBackgroundDrawList();
            if (!draw)
                continue;

            constexpr float radius = 60.f;
            constexpr int segments = 24;

            Color_t fill = C_GET(Color_t, Vars.colInfernoFill);
            Color_t outline = C_GET(Color_t, Vars.colInfernoOutline);

            const int fireCount = std::min(pInferno->GetFireCount(), 32);

            // если inferno потух -> удалить alpha
            if (fireCount <= 0)
            {
                infernoAlpha.erase(pInferno->GetRefEHandle().GetEntryIndex());
                continue;
            }

            int infernoID = pInferno->GetRefEHandle().GetEntryIndex();

            // create alpha
            if (!infernoAlpha.count(infernoID))
                infernoAlpha[infernoID] = 0.f;

            // fade in
            infernoAlpha[infernoID] = std::clamp(
                infernoAlpha[infernoID] + (ImGui::GetIO().DeltaTime * 400.f),
                0.f,
                255.f
            );

            float alphaMul = infernoAlpha[infernoID] / 255.f;

            for (int i = 0; i < fireCount; i++)
            {
                if (!pInferno->IsFireBurning(i))
                    continue;

                Vector_t firePos = pInferno->GetFireOrigin(i);

                if (firePos.IsZero())
                    continue;

                ImVec2 points[24];

                bool valid = true;

                for (int s = 0; s < segments; s++)
                {
                    float angle = ((float)s / (float)segments) * (2.f * IM_PI);

                    Vector_t pos =
                    {
                        firePos.x + cosf(angle) * radius,
                        firePos.y + sinf(angle) * radius,
                        firePos.z
                    };

                    if (!D::WorldToScreen(pos, &points[s]))
                    {
                        valid = false;
                        break;
                    }
                }

                if (!valid)
                    continue;

                // fill
                draw->AddConvexPolyFilled(
                    points,
                    segments,
                    IM_COL32(
                        fill.r,
                        fill.g,
                        fill.b,
                        (int)(25.f * alphaMul)
                    )
                );

                // glow
                draw->AddPolyline(
                    points,
                    segments,
                    IM_COL32(
                        outline.r,
                        outline.g,
                        outline.b,
                        (int)(40.f * alphaMul)
                    ),
                    ImDrawFlags_Closed,
                    4.f
                );

                // outline
                draw->AddPolyline(
                    points,
                    segments,
                    IM_COL32(
                        outline.r,
                        outline.g,
                        outline.b,
                        (int)(180.f * alphaMul)
                    ),
                    ImDrawFlags_Closed,
                    1.5f
                );
            }

            continue;
        }
        if (FNV1A::Hash(pClassInfo->Name) == FNV1A::HashConst("C_SmokeGrenadeProjectile"))
        {
            if (!C_GET(bool, Vars.bSmokeRadius))  
            {
                smokeAlpha.clear();
                continue;
            }

            C_SmokeGrenadeProjectile* pSmoke = reinterpret_cast<C_SmokeGrenadeProjectile*>(pEntity);
            if (!pSmoke)
                continue;

          
            if (!pSmoke->IsActive())
            {
                smokeAlpha.erase(pSmoke->GetRefEHandle().GetEntryIndex());
                continue;
            }

            ImDrawList* draw = ImGui::GetBackgroundDrawList();
            if (!draw)
                continue;

            constexpr float radius = 160.f;  
            constexpr int segments = 32;    

            Color_t fill = C_GET(Color_t, Vars.colSmokeFill);    
            Color_t outline = C_GET(Color_t, Vars.colSmokeOutline); 

            int smokeID = pSmoke->GetRefEHandle().GetEntryIndex();
            Vector_t smokePos = pSmoke->GetSmokeOrigin();

            if (smokePos.IsZero())
                continue;

          
            if (!smokeAlpha.count(smokeID))
                smokeAlpha[smokeID] = 0.f;

            
            smokeAlpha[smokeID] = std::clamp(
                smokeAlpha[smokeID] + (ImGui::GetIO().DeltaTime * 400.f),
                0.f,
                255.f
            );

            float alphaMul = smokeAlpha[smokeID] / 255.f;

           
            ImVec2 points[32];
            bool valid = true;

            for (int s = 0; s < segments; s++)
            {
                float angle = ((float)s / (float)segments) * (2.f * IM_PI);

                Vector_t pos = {
                    smokePos.x + cosf(angle) * radius,
                    smokePos.y + sinf(angle) * radius,
                    smokePos.z
                };

                if (!D::WorldToScreen(pos, &points[s]))
                {
                    valid = false;
                    break;
                }
            }

            if (!valid)
                continue;

   
            draw->AddConvexPolyFilled(
                points,
                segments,
                IM_COL32(
                    fill.r,
                    fill.g,
                    fill.b,
                    (int)(30.f * alphaMul) 
                )
            );

      
            draw->AddPolyline(
                points,
                segments,
                IM_COL32(
                    outline.r,
                    outline.g,
                    outline.b,
                    (int)(50.f * alphaMul)
                ),
                ImDrawFlags_Closed,
                4.f
            );

      
            draw->AddPolyline(
                points,
                segments,
                IM_COL32(
                    outline.r,
                    outline.g,
                    outline.b,
                    (int)(200.f * alphaMul)
                ),
                ImDrawFlags_Closed,
                1.5f
            );

            continue;
        }
        if (FNV1A::Hash(pClassInfo->Name) != FNV1A::HashConst("CCSPlayerController"))
            continue;

        CCSPlayerController* pController = reinterpret_cast<CCSPlayerController*>(pEntity);
        if (!pController)
            continue;

        C_CSPlayerPawn* pPawn = reinterpret_cast<C_CSPlayerPawn*>(I::GameResourceService->pGameEntitySystem->Get(pController->GetPlayerPawnHandle().GetEntryIndex()));
        if (!pPawn)
            continue;
        if (!*reinterpret_cast<uintptr_t**>(pPawn)) {
            continue;


        }
        if (!SDK::LocalPawn->IsOtherEnemy(pPawn))
            continue;
        if (pPawn->GetTeam() == SDK::LocalPawn->GetTeam())
            continue;

        if (pPawn->GetHealth() <= 0)
            continue;

        DrawSkeleton(pPawn);

        ImVec4 box;
        if (!GetEntityBoundingBox(pPawn, &box))
            continue;

        ImVec2 boxMin(floorf(box.x), floorf(box.y));
        ImVec2 boxMax(floorf(box.z), floorf(box.w));

        float boxHeight = boxMax.y - boxMin.y;
        float boxWidth = boxMax.x - boxMin.x;
        if (C_GET(bool, Vars.bBox)) 
        {
            ImDrawList* draw = ImGui::GetBackgroundDrawList();

            float cornerSize = 10.0f;  
            float cornerThickness = 1.0f;
            float outlineThickness = 1.0f; 

      
            Color_t boxColor = C_GET(Color_t, Vars.colBox1);
            ImU32 color = IM_COL32(boxColor.r, boxColor.g, boxColor.b, boxColor.a);
   


            auto DrawCorner = [&](ImVec2 p1, ImVec2 p2, bool horizontal)
                {
              
                    draw->AddLine(p1, p2, color, cornerThickness);
                };

        
            DrawCorner(
                ImVec2(boxMin.x - outlineThickness, boxMin.y - outlineThickness),
                ImVec2(boxMin.x + cornerSize, boxMin.y - outlineThickness),
                true
            );
            DrawCorner(
                ImVec2(boxMin.x - outlineThickness, boxMin.y - outlineThickness),
                ImVec2(boxMin.x - outlineThickness, boxMin.y + cornerSize),
                false
            );

           
            DrawCorner(
                ImVec2(boxMax.x - cornerSize, boxMin.y - outlineThickness),
                ImVec2(boxMax.x + outlineThickness, boxMin.y - outlineThickness),
                true
            );
            DrawCorner(
                ImVec2(boxMax.x + outlineThickness, boxMin.y - outlineThickness),
                ImVec2(boxMax.x + outlineThickness, boxMin.y + cornerSize),
                false
            );

          
            DrawCorner(
                ImVec2(boxMin.x - outlineThickness, boxMax.y - cornerSize),
                ImVec2(boxMin.x - outlineThickness, boxMax.y + outlineThickness),
                false
            );
            DrawCorner(
                ImVec2(boxMin.x - outlineThickness, boxMax.y + outlineThickness),
                ImVec2(boxMin.x + cornerSize, boxMax.y + outlineThickness),
                true
            );

          
            DrawCorner(
                ImVec2(boxMax.x + outlineThickness, boxMax.y - cornerSize),
                ImVec2(boxMax.x + outlineThickness, boxMax.y + outlineThickness),
                false
            );
            DrawCorner(
                ImVec2(boxMax.x - cornerSize, boxMax.y + outlineThickness),
                ImVec2(boxMax.x + outlineThickness, boxMax.y + outlineThickness),
                true
            );
        }
        if (C_GET(bool, Vars.bName)) {
            const char* playerName = pController->GetPlayerName();
            if (playerName && playerName[0] != '\0')
            {
                ImVec2 textSize = ImGui::CalcTextSize(playerName);
                ImVec2 namePos(boxMin.x + (boxWidth / 2) - (textSize.x / 2), boxMin.y - textSize.y - 2);

                Color_t colName = C_GET(Color_t, Vars.colPlayerName);
                Color_t colNameOutline = C_GET(Color_t, Vars.colPlayerNameOutline);

                ImColor nameColor = ImColor((float)colName.r, (float)colName.g, (float)colName.b, (float)colName.a);
                ImColor nameOutline = ImColor((float)colNameOutline.r, (float)colNameOutline.g, (float)colNameOutline.b, (float)colNameOutline.a);

    
                ImGui::GetBackgroundDrawList()->AddText(ImVec2(namePos.x - 1, namePos.y - 1), nameOutline, playerName);
                ImGui::GetBackgroundDrawList()->AddText(ImVec2(namePos.x + 1, namePos.y - 1), nameOutline, playerName);
                ImGui::GetBackgroundDrawList()->AddText(ImVec2(namePos.x - 1, namePos.y + 1), nameOutline, playerName);
                ImGui::GetBackgroundDrawList()->AddText(ImVec2(namePos.x + 1, namePos.y + 1), nameOutline, playerName);

      
                ImGui::GetBackgroundDrawList()->AddText(namePos, nameColor, playerName);
            }
        }

        if (C_GET(bool, Vars.bWeapon)) {
            C_CSWeaponBase* pWeapon = pPawn->GetActiveWeapon();
            if (pWeapon)
            {
                CCSWeaponBaseVData* pWeaponData = pWeapon->GetWeaponVData();
                if (pWeaponData)
                {
                    C_AttributeContainer& attributeManager = pWeapon->GetAttributeManager();
                    C_EconItemView& item = attributeManager.GetItem();

                    int itemIndex = item.GetItemDefinitionIndex();

                    const char* szWeaponName = GetWeaponNameByIndex(itemIndex);
                    ImVec2 textSize = ImGui::CalcTextSize(szWeaponName);
                    ImVec2 weaponPos(boxMin.x + (boxWidth / 2) - (textSize.x / 2), boxMax.y + 2);

                    Color_t colWeapon = C_GET(Color_t, Vars.colWeaponName);
                    Color_t colWeaponOutline = C_GET(Color_t, Vars.colWeaponNameOutline);

                    ImColor weaponColor = ImColor((float)colWeapon.r, (float)colWeapon.g, (float)colWeapon.b, (float)colWeapon.a);
                    ImColor weaponOutline = ImColor((float)colWeaponOutline.r, (float)colWeaponOutline.g, (float)colWeaponOutline.b, (float)colWeaponOutline.a);

          
                    ImGui::GetBackgroundDrawList()->AddText(ImVec2(weaponPos.x - 1, weaponPos.y - 1), weaponOutline, szWeaponName);
                    ImGui::GetBackgroundDrawList()->AddText(ImVec2(weaponPos.x + 1, weaponPos.y - 1), weaponOutline, szWeaponName);
                    ImGui::GetBackgroundDrawList()->AddText(ImVec2(weaponPos.x - 1, weaponPos.y + 1), weaponOutline, szWeaponName);
                    ImGui::GetBackgroundDrawList()->AddText(ImVec2(weaponPos.x + 1, weaponPos.y + 1), weaponOutline, szWeaponName);

                    ImGui::GetBackgroundDrawList()->AddText(weaponPos, weaponColor, szWeaponName);
                }
            }
        }


        if (C_GET(bool, Vars.bHealthBar))
        {
            int currentHealth = pPawn->GetHealth();
            int maxHealth = pPawn->GetMaxHealth();

            if (maxHealth > 0)
            {
                float healthPercent = (float)currentHealth / (float)maxHealth;

                float barHeight = boxHeight;
                float barWidth = 4.0f;
                float barX = boxMax.x + 4.0f;
                float barY = boxMin.y;

                ImDrawList* draw = ImGui::GetBackgroundDrawList();

                ImColor black = ImColor(0.f, 0.f, 0.f, 1.f);

                Color_t colBorder = C_GET(Color_t, Vars.colHealthBarBorder);
                Color_t colText = C_GET(Color_t, Vars.colHealthText);
                Color_t colTextOutline = C_GET(Color_t, Vars.colHealthTextOutline);

                ImColor borderCol = ImColor(colBorder.r, colBorder.g, colBorder.b, colBorder.a);
                ImColor textCol = ImColor(colText.r, colText.g, colText.b, colText.a);
                ImColor textOutline = ImColor(colTextOutline.r, colTextOutline.g, colTextOutline.b, colTextOutline.a);

 
                draw->AddRectFilled(
                    ImVec2(barX, barY),
                    ImVec2(barX + barWidth, barY + barHeight),
                    black
                );

     
                Color_t col1 = C_GET(Color_t, Vars.colHealth1);
                Color_t col2 = C_GET(Color_t, Vars.colHealth2);

                ImColor fillColor1 = ImColor(col1.r, col1.g, col1.b, col1.a);
                ImColor fillColor2 = ImColor(col2.r, col2.g, col2.b, col2.a);

                float filledHeight = barHeight * healthPercent;
                float fillTop = barY + (barHeight - filledHeight);
                float fillBottom = barY + barHeight;

                draw->AddRectFilledMultiColor(
                    ImVec2(barX, fillTop),
                    ImVec2(barX + barWidth, fillBottom),
                    fillColor1,
                    fillColor1, 
                    fillColor2, 
                    fillColor2  
                );

                draw->AddRect(
                    ImVec2(barX, barY),
                    ImVec2(barX + barWidth, barY + barHeight),
                    borderCol
                );

        
                if (C_GET(bool, Vars.bHealthText))
                {
                    char text[16];
                    snprintf(text, sizeof(text), "%d", currentHealth);

                    ImVec2 textSize = ImGui::CalcTextSize(text);

                    ImVec2 textPos(
                        barX + barWidth + 2,
                        barY + (barHeight * 0.5f) - (textSize.y * 0.5f)
                    );

                    draw->AddText(ImVec2(textPos.x - 1, textPos.y - 1), textOutline, text);
                    draw->AddText(ImVec2(textPos.x + 1, textPos.y - 1), textOutline, text);
                    draw->AddText(ImVec2(textPos.x - 1, textPos.y + 1), textOutline, text);
                    draw->AddText(ImVec2(textPos.x + 1, textPos.y + 1), textOutline, text);

                    draw->AddText(textPos, textCol, text);
                }
            }
        }

        // Snaplines
        if (C_GET(bool, Vars.bSnaplines))
        {
            ImDrawList* draw = ImGui::GetBackgroundDrawList();
            Color_t colSnap = C_GET(Color_t, Vars.colSnaplines);
            ImU32 snapCol = IM_COL32(colSnap.r, colSnap.g, colSnap.b, colSnap.a);
            ImVec2 screenBottom(ImGui::GetIO().DisplaySize.x * 0.5f, ImGui::GetIO().DisplaySize.y);
            draw->AddLine(screenBottom, ImVec2(boxMin.x + boxWidth * 0.5f, boxMax.y), snapCol, 1.f);
        }

        // Head dot
        if (C_GET(bool, Vars.bHeadDot))
        {
            Vector_t headPos = GetBonePos(pPawn, BoneID::Head);
            if (!headPos.IsZero())
            {
                ImVec2 headScreen;
                if (D::WorldToScreen(headPos, &headScreen))
                {
                    Color_t colDot = C_GET(Color_t, Vars.colHeadDot);
                    ImGui::GetBackgroundDrawList()->AddCircleFilled(headScreen,
                        3.f, IM_COL32(colDot.r, colDot.g, colDot.b, colDot.a));
                }
            }
        }

        // Ammo bar (bottom)
        if (C_GET(bool, Vars.bAmmoBar))
        {
            C_CSWeaponBase* pWeaponAmmo = pPawn->GetActiveWeapon();
            if (pWeaponAmmo)
            {
                int clip = pWeaponAmmo->GetClip1();
                CCSWeaponBaseVData* pVD = pWeaponAmmo->GetWeaponVData();
                if (pVD && !pVD->IsMeleeWeapon() && clip >= 0)
                {
                    int maxClip = 30;
                    float cycleTime = pVD->GetCycleTime();
                    if (cycleTime > 0.f)
                        maxClip = (int)(1.f / cycleTime) + clip;
                    if (maxClip < clip) maxClip = clip;
                    if (maxClip <= 0) maxClip = 1;

                    float ammoFrac = (float)clip / (float)maxClip;
                    if (ammoFrac > 1.f) ammoFrac = 1.f;

                    ImDrawList* draw = ImGui::GetBackgroundDrawList();
                    float aBarY = boxMax.y + 16.f;
                    float aBarH = 3.f;
                    draw->AddRectFilled(ImVec2(boxMin.x, aBarY), ImVec2(boxMax.x, aBarY + aBarH),
                        IM_COL32(0, 0, 0, 180));
                    draw->AddRectFilled(ImVec2(boxMin.x, aBarY),
                        ImVec2(boxMin.x + boxWidth * ammoFrac, aBarY + aBarH),
                        IM_COL32(255, 180, 0, 255));
                }
            }
        }

        // Armor bar (left side)
        if (C_GET(bool, Vars.bArmorBar))
        {
            int armor = pController->GetPawnArmor();
            if (armor > 0)
            {
                float armorFrac = (float)armor / 100.f;
                if (armorFrac > 1.f) armorFrac = 1.f;

                Color_t colArmor = C_GET(Color_t, Vars.colArmorBar);
                ImDrawList* draw = ImGui::GetBackgroundDrawList();
                float arBarX = boxMin.x - 8.f;
                float arBarW = 3.f;
                float filledH = boxHeight * armorFrac;
                float fillTopY = boxMin.y + (boxHeight - filledH);

                draw->AddRectFilled(ImVec2(arBarX, boxMin.y),
                    ImVec2(arBarX + arBarW, boxMax.y), IM_COL32(0, 0, 0, 180));
                draw->AddRectFilled(ImVec2(arBarX, fillTopY),
                    ImVec2(arBarX + arBarW, boxMax.y),
                    IM_COL32(colArmor.r, colArmor.g, colArmor.b, colArmor.a));
            }
        }

        // Flags
        if (C_GET(bool, Vars.bFlags))
        {
            ImDrawList* draw = ImGui::GetBackgroundDrawList();
            float flagX = boxMax.x + 14.f;
            float flagY = boxMin.y;
            float lineH = 10.f;

            if (C_GET(bool, Vars.bFlagArmor) && pController->GetPawnArmor() > 0)
            {
                bool helmet = pController->IsPawnHasHelmet();
                draw->AddText(ImVec2(flagX, flagY), IM_COL32(200, 200, 200, 255), helmet ? "HK" : "K");
                flagY += lineH;
            }

            if (C_GET(bool, Vars.bFlagScoped) && pPawn->IsScoped())
            {
                draw->AddText(ImVec2(flagX, flagY), IM_COL32(100, 180, 255, 255), "ZOOM");
                flagY += lineH;
            }

            if (C_GET(bool, Vars.bFlagDefusing) && pPawn->IsDefusing())
            {
                draw->AddText(ImVec2(flagX, flagY), IM_COL32(0, 255, 0, 255), "DEF");
                flagY += lineH;
            }

            if (C_GET(bool, Vars.bFlagKit) && pController->IsPawnHasDefuser())
            {
                draw->AddText(ImVec2(flagX, flagY), IM_COL32(100, 200, 255, 255), "KIT");
                flagY += lineH;
            }

            if (C_GET(bool, Vars.bFlagFlashed) && pPawn->GetFlashDuration() > 0.f)
            {
                draw->AddText(ImVec2(flagX, flagY), IM_COL32(255, 255, 0, 255), "FLASH");
                flagY += lineH;
            }
        }

        // Distance
        if (C_GET(bool, Vars.bDistance))
        {
            CGameSceneNode* pNodeDist = pPawn->GetGameSceneNode();
            if (pNodeDist)
            {
                Vector_t localPos = SDK::LocalPawn->GetGameSceneNode()->GetAbsOrigin();
                Vector_t enemyPos = pNodeDist->GetAbsOrigin();
                float dist = (enemyPos - localPos).Length() * 0.0254f;

                char distBuf[16];
                snprintf(distBuf, sizeof(distBuf), "%.0fm", dist);
                ImVec2 distSz = ImGui::CalcTextSize(distBuf);
                float dY = boxMax.y + 2.f;
                if (C_GET(bool, Vars.bWeapon)) dY += 12.f;
                if (C_GET(bool, Vars.bAmmoBar)) dY += 6.f;

                ImGui::GetBackgroundDrawList()->AddText(
                    ImVec2(boxMin.x + (boxWidth - distSz.x) * 0.5f, dY),
                    IM_COL32(180, 180, 180, 255), distBuf);
            }
        }

    }
    }


   