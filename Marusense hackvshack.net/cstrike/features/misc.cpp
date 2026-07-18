#include "misc.h"
#include "misc/movement.h"

void F::MISC::OnMove(CUserCmd* pCmd, CBaseUserCmdPB* pBaseCmd, CCSPlayerController* pLocalController, C_CSPlayerPawn* pLocalPawn)
{
	MOVEMENT::BunnyHop(pCmd, pBaseCmd, pLocalPawn);
	MOVEMENT::EdgeBug(pCmd, pBaseCmd, pLocalPawn);
	MOVEMENT::JumpBug(pCmd, pBaseCmd, pLocalPawn);
}
