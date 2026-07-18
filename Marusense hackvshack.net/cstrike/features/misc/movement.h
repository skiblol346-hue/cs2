#pragma once

class CUserCmd;
class CBaseUserCmdPB;
class C_CSPlayerPawn;

namespace F::MISC::MOVEMENT
{
	void BunnyHop(CUserCmd* pCmd, CBaseUserCmdPB* pBaseCmd, C_CSPlayerPawn* pLocalPawn);
	void EdgeBug(CUserCmd* pCmd, CBaseUserCmdPB* pBaseCmd, C_CSPlayerPawn* pLocalPawn);
	void JumpBug(CUserCmd* pCmd, CBaseUserCmdPB* pBaseCmd, C_CSPlayerPawn* pLocalPawn);
}
