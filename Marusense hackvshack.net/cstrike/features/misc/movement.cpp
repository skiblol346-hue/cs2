#include "movement.h"
#include <Windows.h>
#include "../../core/variables.h"
#include "../../sdk/datatypes/usercmd.h"
#include "../../sdk/entity.h"
#include "../../sdk/const.h"
#include "../../utilities/inputsystem.h"
#include <cmath>

void F::MISC::MOVEMENT::BunnyHop(CUserCmd* pCmd, CBaseUserCmdPB* pBaseCmd, C_CSPlayerPawn* pLocalPawn)
{
	if (!C_GET(bool, Vars.bBhop))
		return;

	constexpr std::uint64_t kJumpMask = IN_JUMP;

	const std::uint64_t cmdButtons = pCmd->nButtons.nValue;
	const std::uint64_t baseButtons = (pBaseCmd && pBaseCmd->pInButtonState) ? pBaseCmd->pInButtonState->nValue : 0ULL;
	if (((cmdButtons | baseButtons) & kJumpMask) == 0ULL)
		return;

	const int moveType = pLocalPawn->GetMoveType();
	if (moveType == MOVETYPE_NOCLIP || moveType == MOVETYPE_LADDER)
		return;

	const std::uint32_t flags = pLocalPawn->GetFlags();
	if ((flags & FL_ONGROUND) != 0U)
		return;

	pCmd->nButtons.nValue &= ~kJumpMask;
	pCmd->nButtons.nValueChanged &= ~kJumpMask;
	pCmd->nButtons.nValueScroll &= ~kJumpMask;

	if (pBaseCmd && pBaseCmd->pInButtonState)
	{
		pBaseCmd->pInButtonState->nValue &= ~kJumpMask;
		pBaseCmd->pInButtonState->nValueChanged &= ~kJumpMask;
		pBaseCmd->pInButtonState->nValueScroll &= ~kJumpMask;
		pBaseCmd->pInButtonState->SetBits(
			BUTTON_STATE_PB_BITS_BUTTONSTATE1 |
			BUTTON_STATE_PB_BITS_BUTTONSTATE2 |
			BUTTON_STATE_PB_BITS_BUTTONSTATE3
		);
	}

	if (pBaseCmd && pBaseCmd->subtickMovesField.pRep)
	{
		for (int i = 0; i < pBaseCmd->subtickMovesField.nCurrentSize; ++i)
		{
			CSubtickMoveStep* step = pBaseCmd->subtickMovesField.pRep->tElements[i];
			if (!step || (step->nButton & kJumpMask) == 0ULL)
				continue;

			step->nButton &= ~kJumpMask;
			step->bPressed = false;
			step->SetBits(MOVESTEP_BITS_BUTTON | MOVESTEP_BITS_PRESSED);
		}
	}

	if (pBaseCmd)
		pBaseCmd->SetBits(BASE_BITS_BUTTONPB);
}

void F::MISC::MOVEMENT::EdgeBug(CUserCmd* pCmd, CBaseUserCmdPB* pBaseCmd, C_CSPlayerPawn* pLocalPawn)
{
	if (!C_GET(bool, Vars.bEdgeBug))
		return;

	const unsigned int nKey = C_GET(unsigned int, Vars.nEdgeBugKey);
	if (nKey != 0 && !IPT::IsKeyDown(nKey))
		return;

	const std::uint32_t flags = pLocalPawn->GetFlags();
	if (flags & FL_ONGROUND)
		return;

	const int moveType = pLocalPawn->GetMoveType();
	if (moveType == MOVETYPE_LADDER || moveType == MOVETYPE_NOCLIP)
		return;

	const Vector_t vel = pLocalPawn->GetVecVelocity();
	if (vel.z >= 0.f)
		return;

	static bool bEdgeBugging = false;
	static float flLastZVel = 0.f;

	float flCurZ = vel.z;
	if (flLastZVel < -7.f && std::floor(flCurZ) == -7.f && !(flags & FL_ONGROUND))
	{
		bEdgeBugging = true;

		if (pBaseCmd)
		{
			pBaseCmd->flForwardMove = 0.f;
			pBaseCmd->flSideMove = 0.f;
			pBaseCmd->nMousedX = 0;
			pBaseCmd->nMousedY = 0;
			pBaseCmd->SetBits(BASE_BITS_FORWARDMOVE | BASE_BITS_LEFTMOVE | BASE_BITS_MOUSEDX | BASE_BITS_MOUSEDY);
		}

		pCmd->nButtons.nValue &= ~(IN_FORWARD | IN_BACK | IN_MOVELEFT | IN_MOVERIGHT);
		pCmd->nButtons.nValue |= IN_DUCK;
	}

	flLastZVel = flCurZ;

	if (bEdgeBugging)
	{
		if (flags & FL_ONGROUND || moveType == MOVETYPE_LADDER)
		{
			bEdgeBugging = false;
			return;
		}

		if (pBaseCmd)
		{
			pBaseCmd->flForwardMove = 0.f;
			pBaseCmd->flSideMove = 0.f;
			pBaseCmd->SetBits(BASE_BITS_FORWARDMOVE | BASE_BITS_LEFTMOVE);
		}
		pCmd->nButtons.nValue &= ~(IN_FORWARD | IN_BACK | IN_MOVELEFT | IN_MOVERIGHT);
		pCmd->nButtons.nValue |= IN_DUCK;
	}
}

void F::MISC::MOVEMENT::JumpBug(CUserCmd* pCmd, CBaseUserCmdPB* pBaseCmd, C_CSPlayerPawn* pLocalPawn)
{
	if (!C_GET(bool, Vars.bJumpBug))
		return;

	const unsigned int nKey = C_GET(unsigned int, Vars.nJumpBugKey);
	if (nKey != 0 && !IPT::IsKeyDown(nKey))
		return;

	if (!(pCmd->nButtons.nValue & IN_JUMP))
		return;

	const int moveType = pLocalPawn->GetMoveType();
	if (moveType == MOVETYPE_LADDER || moveType == MOVETYPE_NOCLIP)
		return;

	const std::uint32_t flags = pLocalPawn->GetFlags();
	const Vector_t vel = pLocalPawn->GetVecVelocity();

	static float flLastZ = 0.f;
	const float flCurZ = vel.z;

	if (!(flags & FL_ONGROUND) && flLastZ < 0.f && flCurZ > flLastZ && flCurZ < 0.f)
	{
		pCmd->nButtons.nValue |= IN_DUCK;
		pCmd->nButtons.nValue &= ~IN_JUMP;

		if (pBaseCmd)
		{
			if (pBaseCmd->pInButtonState)
			{
				pBaseCmd->pInButtonState->nValue |= IN_DUCK;
				pBaseCmd->pInButtonState->nValue &= ~IN_JUMP;
				pBaseCmd->pInButtonState->SetBits(
					BUTTON_STATE_PB_BITS_BUTTONSTATE1 |
					BUTTON_STATE_PB_BITS_BUTTONSTATE2 |
					BUTTON_STATE_PB_BITS_BUTTONSTATE3
				);
			}
			pBaseCmd->SetBits(BASE_BITS_BUTTONPB);
		}
	}

	flLastZ = flCurZ;
}
