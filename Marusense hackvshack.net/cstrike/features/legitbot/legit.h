
// Исправленный полный код:
#pragma once
#include <random>
#include <thread>
#include <chrono>
#include "../../features/ragebot/Rage.h"
#include "../../sdk/entity.h"
#include "../../sdk/interfaces/cgameentitysystem.h"

#include "../../sdk/interfaces/cgametracemanager.h"

#include "../../sdk/datatypes/usercmd.h"
#include <sstream>
#include "../../core/interfaces.h"
#include "../../utilities/inputsystem.h"
#include "../../sdk/interfaces/ccsgoinput.h"

#include "../../core/variables.h"
#include "../../core/sdk.h"

#include <numbers>

#include <cstring>
#ifndef _USE_MATH_DEFINES
#define _USE_MATH_DEFINES
#endif
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
class CUserCmd;
class CBaseUserCmdPB;
class CCSGOInputHistoryEntryPB;

class CCSPlayerController;
class C_CSPlayerPawn;

struct QAngle_t;

namespace F::LEGITBOT::LEGIT
{
    bool IsBulletHitChance(C_CSPlayerPawn* pLocalPawn,
        C_CSPlayerPawn* pTargetPawn,
        const Vector_t& vecTargetPos,  // ← НОВЫЙ ПАРАМЕТР!
        CBaseUserCmdPB* pUserCmd,
        CCSPlayerController* pLocalController,
        float iHitChance);
    
    bool IsSpreadValid(C_CSPlayerPawn* local, C_CSPlayerPawn* target, const Vector_t& targetpos, CBaseUserCmdPB* cmd, CCSPlayerController* controller);
    void InitSpreadFunctions();
    void OnMove(CUserCmd* pCmd, CBaseUserCmdPB* pBaseCmd, CCSPlayerController* pLocalController, C_CSPlayerPawn* pLocalPawn);
    void TriggerBot(CUserCmd* pCmd, CBaseUserCmdPB* pUserCmd, C_CSPlayerPawn* pLocalPawn, CCSPlayerController* pLocalController);
  
}