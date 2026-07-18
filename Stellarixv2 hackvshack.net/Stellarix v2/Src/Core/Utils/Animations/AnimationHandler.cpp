#include "AnimationHandler.hpp"
#include <Core/Sdk/Datatypes/Easing.hpp>

void AnimationHandler_t::Update( const float flDeltaTime, const float flDuration )
{
	if ( fnEaseIn == nullptr )
		fnEaseIn = &EASING::InSine;

	if ( fnEaseOut == nullptr )
		fnEaseOut = &EASING::OutSine;

	// Reset the elapsed time if the bool switches
	if ( bSwitch != bLastSwitch )
		flElapsedTime = 0;

	flElapsedTime = MATH::Max( 0.0f, MATH::Min( flElapsedTime, flDuration ) );
	float flTime = flElapsedTime / flDuration;

	// Determine the initial and target value based on the current state
	float flInitialValue = bSwitch ? 0.1f : flValue;
	float flTargetValue = bSwitch ? 1.0f : 0.1f; /*(1.0f is max value)*/

	// Select the appropriate easing function based on the current state
	EasingFunction_t fnCurrentEase = bSwitch ? fnEaseIn : fnEaseOut;

	// Apply the appropriate easing function based on fade-in or fade-out (with lerping, which is basically what's the math were doing)
	flValue = ( flInitialValue + ( flTargetValue - flInitialValue ) ) * ( float )fnCurrentEase( flTime );
	flValue = MATH::Clamp( flValue, 0.1f, 1.0f );

	flElapsedTime += flDeltaTime;
	bLastSwitch = bSwitch;
}
