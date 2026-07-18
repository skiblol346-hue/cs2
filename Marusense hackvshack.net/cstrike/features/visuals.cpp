#include "visuals.h"

// used: source sdk

#include "../sdk/entity.h"

// used: overlay
#include "visuals/overlay.h"
#include "visuals/chamss.h"

#include "../core/sdk.h"

using namespace F;

bool F::VISUALS::Setup()
{

    CHAMS::Initialize();
	return true;
}

void F::VISUALS::OnDestroy()
{
    CHAMS::Destroy();
}

void F::VISUALS::OnRender()
{
    F::VISUALS::ESP::Render();
}
bool F::VISUALS::OnDrawObject(void* pAnimatableSceneObjectDesc, void* pDx11, CMeshData* arrMeshDraw, int nDataCount, void* pSceneView, void* pSceneLayer, void* pUnk, void* pUnk2)
{

    return   CHAMS::OnDrawObject(pAnimatableSceneObjectDesc, pDx11, arrMeshDraw, nDataCount, pSceneView, pSceneLayer, pUnk, pUnk2);
}

void VISUALS::OnFrame(const int nStage)
{
    if (!D::bInitialized)
        return;





}