#pragma once
#include <Core/Sdk/Datatypes/QAngle.hpp>

class CCSGOInput
{
public:
	auto GetViewAngle( ) -> QAngle_t;
	auto SetViewAngle( QAngle_t& angView ) -> void;
};
