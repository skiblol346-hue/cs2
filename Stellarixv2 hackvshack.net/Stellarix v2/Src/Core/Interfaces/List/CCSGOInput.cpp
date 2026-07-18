#include "CCSGOInput.hpp"
#include <Core/Engine/FuncrionCS2.hpp>

auto CCSGOInput::GetViewAngle( ) -> QAngle_t
{
	return *reinterpret_cast< QAngle_t* >( FunctionsCS2::FnGetViewAngles( this, 0 ) );
}

auto CCSGOInput::SetViewAngle( QAngle_t& angView ) -> void
{
	FunctionsCS2::FnSetViewAngles( this, 0, std::ref( angView ) );
}
