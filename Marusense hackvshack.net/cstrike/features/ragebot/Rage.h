#pragma once

#include "../../core/sdk.h"
#include "../../sdk/entity.h"
#include "../../sdk/interfaces/ccsgoinput.h"
#include "../../utilities/math.h"

class CUserCmd;
class CBaseUserCmdPB;
class CCSPlayerController;
class C_CSPlayerPawn;

struct Target_t
{
    CCSPlayerController* pController = nullptr;
    C_CSPlayerPawn* pPawn = nullptr;
    Vector_t vecHitPos = {};
    int iDamage = 0;
    float flFov = 0.f;
    int iHitbox = -1;
};

namespace F::RAGEBOT
{
    void Init();
   bool CheckPoint(const Vector_t& vecStart, const Vector_t& vecPoint,
        C_CSPlayerPawn* pLocal, C_CSPlayerPawn* pTarget, int& iDamage,
        bool bAutoWall, int iMinDamage = 0);
    void OnMove(CUserCmd* pCmd, CBaseUserCmdPB* pBaseCmd,

        CCSPlayerController* pLocalController,
        C_CSPlayerPawn* pLocalPawn);

    void Reset();

    namespace Internal
    {
        inline Target_t g_target = {};
        inline bool g_bActive = false;
    }
}