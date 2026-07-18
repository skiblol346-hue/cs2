#pragma once
#include <windows.h>
#include "common.h"
#include <sstream>
#include <chrono>
#include "sdk/datatypes/vector.h"

class CUserCmd;
class CBaseUserCmdPB;
class CCSPlayerController;
class CMeshData;
class C_CSPlayerPawn;
namespace F
{
	bool Setup();
	void Destroy();
	bool OnDrawObject(void* pAnimatableSceneObjectDesc, void* pDx11, CMeshData* arrMeshDraw, int nDataCount, void* pSceneView, void* pSceneLayer, void* pUnk, void* pUnk2);
	void OnPresent();
	void OnFrameStageNotify(int nStage);
	
	void OnCreateMove(CUserCmd* pCmd, CBaseUserCmdPB* pBaseCmd, CCSPlayerController* pLocalController, C_CSPlayerPawn* pLocalPawn);
	void OnDamageDealt(const Vector_t& pos, int damage);
}
