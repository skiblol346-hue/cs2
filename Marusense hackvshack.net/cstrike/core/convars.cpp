// used: [stl] vector
#include <vector>
// used: [stl] find_if
#include <algorithm>

#include "convars.h"

// used: convar interface
#include "interfaces.h"
#include "../sdk/interfaces/ienginecvar.h"
// used: l_print
#include "../utilities/log.h"

// used: getworkingpath
#include "../core.h"

bool CONVAR::Setup()
{
	bool bSuccess = true;

	m_pitch = I::Cvar->Find(FNV1A::HashConst("m_pitch"));
	bSuccess &= m_pitch != nullptr;

	m_yaw = I::Cvar->Find(FNV1A::HashConst("m_yaw"));
	bSuccess &= m_yaw != nullptr;

	sensitivity = I::Cvar->Find(FNV1A::HashConst("sensitivity"));
	bSuccess &= sensitivity != nullptr;

	game_type = I::Cvar->Find(FNV1A::HashConst("game_type"));
	bSuccess &= game_type != nullptr;

	game_mode = I::Cvar->Find(FNV1A::HashConst("game_mode"));
	bSuccess &= game_mode != nullptr;

	mp_teammates_are_enemies = I::Cvar->Find(FNV1A::HashConst("mp_teammates_are_enemies"));
	bSuccess &= mp_teammates_are_enemies != nullptr;

	sv_autobunnyhopping = I::Cvar->Find(FNV1A::HashConst("sv_autobunnyhopping"));
	bSuccess &= sv_autobunnyhopping != nullptr;


	return bSuccess;
}