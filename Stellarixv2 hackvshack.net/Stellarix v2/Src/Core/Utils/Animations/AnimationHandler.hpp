#pragma once

typedef double ( *EasingFunction_t )( double );
struct AnimationHandler_t
{
	// default: ease::in/outsine
	AnimationHandler_t( EasingFunction_t fnIn = nullptr, EasingFunction_t fnOut = nullptr ) :
		fnEaseIn( fnIn ), fnEaseOut( fnOut ), bSwitch( false ), bLastSwitch( false ), flElapsedTime( 0.f ), flValue( 0.1f )
	{
	};
	~AnimationHandler_t( ) = default;

	// Has to be called every frame
	void Update( const float flDeltaTime, const float flDuration );

	// Get the current value multiplied by a scale
	float GetValue( float flScale = 1.0f )
	{
		return flValue * flScale;
	}

	const bool GetSwitch( ) const
	{
		return bSwitch;
	}

	// switch state
	void Switch( )
	{
		bSwitch = !bSwitch;
	}

	void SetSwitch( const bool bState )
	{
		bSwitch = bState;
	}

private:
	// Set to true for ease-in animation, false for ease-out
	bool bSwitch = 0;
	bool bLastSwitch = bSwitch;

	float flElapsedTime = 0.f;

	// Current value of the animation
	float flValue = 0.1f;

	// Ease in and out functions Declaration
	EasingFunction_t fnEaseIn = nullptr;
	EasingFunction_t fnEaseOut = nullptr;
};