#include "Hook_FrameStageNotify.hpp"
#include <Core/StellarixClient.hpp>
#include <Core/Interfaces/InterfaceManager.hpp>
#include <Core/Interfaces/List/IEngineClient.hpp>

auto __fastcall Hook_FrameStageNotify( void* pSource2Client, int nFrameStage ) -> void
{
	GetStellarixClient( )->OnFrameStageNotifyHook( pSource2Client, nFrameStage );
}
