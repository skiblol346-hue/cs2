#pragma once
#include "Config.hpp"
#include <Core/Sdk/Datatypes/Color.hpp>

#define CFG_UNDEFINE_INT -250

using MiscDpiScale_t = int;
enum EMiscDpiScale : MiscDpiScale_t
{
	MISC_DPISCALE_75 = 0,
	MISC_DPISCALE_DEFAULT,
	MISC_DPISCALE_125,
	MISC_DPISCALE_150,
	MISC_DPISCALE_175,
	MISC_DPISCALE_200,
	MISC_DPISCALE_MAX
};

enum EPlayerEspFlags : unsigned int
{
	kFlagsNone = 0U,
	kFlagsPing = 1 << 0,
	kFlagsZoom = 1 << 1,
	kFlagsAmmunition = 1 << 2,
	kFlagsC4Owner = 1 << 3,
	kFlagsHelmetKit = 1 << 4,
	kFlagsDefuserKit = 1 << 5,
	kFlagsAll = kFlagsPing | kFlagsZoom | kFlagsAmmunition | kFlagsC4Owner | kFlagsHelmetKit | kFlagsDefuserKit,
};

enum EAimHitboxes : unsigned int
{
	kAimHitboxesNone = 0U,
	kAimHitboxesHead = 1 << 0,
	kAimHitboxesChest = 1 << 1,
	kAimHitboxesStomach = 1 << 2,
	kAimHitboxesCenter = 1 << 3,
	kAimHitboxesPelvis = 1 << 4,
	kAimHitboxesHands = 1 << 5,
	kAimHitboxesLegs = 1 << 6,
	kAimHitboxesAll = kAimHitboxesHead | kAimHitboxesChest | kAimHitboxesStomach | kAimHitboxesCenter | kAimHitboxesPelvis | kAimHitboxesHands | kAimHitboxesLegs,
};

enum EChamsMaterials : int
{
	kMaterialDisabled = 0,
	kMaterialIlluminate,
	kMaterialGlow,
	kMaterialPrimary,
	kMaterialLatex,
	kMaterialMetalic,
	kMaterialMax
};

enum class EKeyBindMode : int
{
	HOLD = 0,
	TOGGLE
};

struct KeyBind_t
{
	KeyBind_t( ) = default;

	explicit constexpr KeyBind_t( const bool bShowInBindlist, const unsigned int uKey = 0U, const EKeyBindMode nMode = EKeyBindMode::HOLD ) :
		bShowInBindlist( bShowInBindlist ), uKey( uKey ), nMode( nMode )
	{
	}

	explicit constexpr KeyBind_t( const bool bShowInBindlist, const unsigned int uKey = 0U, const int nMode = ( int )EKeyBindMode::HOLD ) :
		bShowInBindlist( bShowInBindlist ), uKey( uKey ), nMode( ( EKeyBindMode )nMode )
	{
	}

	bool bEnable = false;

	bool bShowInBindlist = false;
	unsigned int uKey = 0U;
	EKeyBindMode nMode = EKeyBindMode::HOLD;
};

using AntiAimPitch_t = int;
enum EAntiAimPitch : AntiAimPitch_t
{
	kAntiAimPitchDisabled = 0,
	kAntiAimPitchOffset,
	kAntiAimPitchRandom,
	kAntiAimPitchJitter,
	kAntiAimPitchDown,
	kAntiAimPitchUp
};

using AntiAimYawModifier_t = int;
enum EAntiAimYawModifier : AntiAimYawModifier_t
{
	kAntiAimYawModifierDisabled = 0,
	kAntiAimYawModifierRandom,
	kAntiAimYawModifierJitter
};

struct Variables_t
{
#pragma region rage
	C_ADD_VARIABLE( bool, bEnableRage, false );
	C_ADD_VARIABLE( bool, bEnableSilentAimRage, false );
	C_ADD_VARIABLE( float, flFieldOfViewRage, 1.f );

	C_ADD_VARIABLE_ARRAY( unsigned int, 50, nHitboxesRage, kAimHitboxesNone, kAimHitboxesNone );
	C_ADD_VARIABLE_ARRAY( unsigned int, 50, nMultipointHitboxesRage, kAimHitboxesNone, kAimHitboxesNone );
#pragma endregion

#pragma region anti_aim
	C_ADD_VARIABLE( bool, bEnableAntiAim, false );
	C_ADD_VARIABLE( int, nAntiAimPitch, kAntiAimPitchDisabled );              // AntiAimPitch_t
	C_ADD_VARIABLE( float, flAntiAimPitchOffset, 0.f );

	C_ADD_VARIABLE( float, flAntiAimYawOffset, 0.f );
	C_ADD_VARIABLE( int, nAntiAimYawModifier, kAntiAimYawModifierDisabled );  // AntiAimYaw_t
	C_ADD_VARIABLE( float, flAntiAimYawModifierOffset, 0.f );
#pragma endregion

#pragma region esp
	C_ADD_VARIABLE( bool, bPlayerEsp, false );
	C_ADD_VARIABLE( bool, bPlayerEspBox, false );
	C_ADD_VARIABLE( bool, bPlayerEspHealthBar, false );
	C_ADD_VARIABLE( bool, bPlayerEspBacktrack, false );
	C_ADD_VARIABLE( float, flPlayerEspHealthBarAnimSpeed, 1.5f );
	C_ADD_VARIABLE( unsigned int, nPlayerEspFlags, kFlagsNone );
	C_ADD_VARIABLE( Color_t, colPlayerEspBoxColor, Color_t( 235, 235, 235 ) );
	C_ADD_VARIABLE( Color_t, colPlayerEspHealthBarColor, Color_t( 235, 235, 235 ) );
	C_ADD_VARIABLE( Color_t, colPlayerEspBacktrackColor, Color_t( 235, 235, 235 ) );


	C_ADD_VARIABLE( bool, bDroppedWeaponEsp, false );
	C_ADD_VARIABLE( bool, bDroppedWeaponEspBox, false );
	C_ADD_VARIABLE( Color_t, colEspC4Color, Color_t( 240, 136, 38 ) );
	C_ADD_VARIABLE( Color_t, colDroppedWeaponColor, Color_t( 235, 235, 235 ) );
#pragma endregion


#pragma region world
	C_ADD_VARIABLE( Color_t, colWorldColor, Color_t( 255, 255, 255 ) );
	C_ADD_VARIABLE( Color_t, colLightingColor, Color_t( 255, 255, 255, 255 ) );
	C_ADD_VARIABLE( float, flLightingIntensity, 1.f );
#pragma endregion


#pragma region watermark
	C_ADD_VARIABLE( bool, bWatermark, true );
	C_ADD_VARIABLE( bool, bWatermarkUserName, false );
	C_ADD_VARIABLE( bool, bWatermarkFps, false );
	C_ADD_VARIABLE( bool, bWatermarkClock, true );
	C_ADD_VARIABLE( bool, bWatermarkPing, false );
	C_ADD_VARIABLE( bool, bWatermarkConfig, true );
	C_ADD_VARIABLE( bool, bWatermarkFixedPos, true );
#pragma endregion


#pragma region misc
	C_ADD_VARIABLE( bool, bBombInfoPanel, false );
	C_ADD_VARIABLE( bool, bSpectatorsList, false );

	C_ADD_VARIABLE( bool, bRemoveLegs, false );
	C_ADD_VARIABLE( bool, bRemoveScope, false );
	C_ADD_VARIABLE( bool, bRemoveTeamIntro, false );

	C_ADD_VARIABLE( bool, bForceCrosshair, false );
	C_ADD_VARIABLE( bool, bMatchAutoAccept, false );

	C_ADD_VARIABLE( bool, bHitMarkerCrosshair, false );
	C_ADD_VARIABLE( bool, bHitGlobalDamageHitMarker, false );
	C_ADD_VARIABLE( bool, bHitHealthBoostEffect, false );
	C_ADD_VARIABLE( int, iHitSound, -1 );

	C_ADD_VARIABLE( float, flAspectRatio, 0.f );

	C_ADD_VARIABLE( int, iCustomAgent, -1 );


	C_ADD_VARIABLE( bool, bBulletBeam, false );
	C_ADD_VARIABLE( Color_t, colBeamColor, Color_t( 210, 134, 240 ) );


	//C_ADD_VARIABLE_VECTOR( KeyBind_t, bindsThirdPerson );
	C_ADD_VARIABLE( KeyBind_t, bindThirdPerson, KeyBind_t() );
	C_ADD_VARIABLE( int, iThirdPersonDistance, 0 );


	C_ADD_VARIABLE( bool, bPlayerTrail, false );
	C_ADD_VARIABLE( Color_t, colPlayerTrailColor, Color_t( 53, 66, 255, 185 ) );


	C_ADD_VARIABLE( float, flViewModelOffsetX, CFG_UNDEFINE_INT );
	C_ADD_VARIABLE( float, flViewModelOffsetY, CFG_UNDEFINE_INT );
	C_ADD_VARIABLE( float, flViewModelOffsetZ, CFG_UNDEFINE_INT );
	C_ADD_VARIABLE( float, flViewModelFov, CFG_UNDEFINE_INT );
	C_ADD_VARIABLE( float, flFov, 90.f );
	C_ADD_VARIABLE( bool, bForceFovInZoom, false );
	C_ADD_VARIABLE( float, flFullScreenGamma, 2.2f );
#pragma endregion


#pragma region chams
	C_ADD_VARIABLE( int, eEnemyVisiblePlayerChams, kMaterialDisabled );
	C_ADD_VARIABLE( Color_t, colEnemyVisiblePlayerChamsColor, Color_t( 53, 66, 255, 185 ) );
	C_ADD_VARIABLE( int, eEnemyInvisiblePlayerChams, kMaterialDisabled );
	C_ADD_VARIABLE( Color_t, colEnemyInvisiblePlayerChamsColor, Color_t( 255, 223, 101, 28 ) );


	C_ADD_VARIABLE( int, eLocalPlayerChams, kMaterialDisabled );
	C_ADD_VARIABLE( Color_t, colLocalPlayerChamsColor, Color_t( 138, 168, 207, 200 ) );
	C_ADD_VARIABLE( int, eLocalViewModelChams, kMaterialDisabled );
	C_ADD_VARIABLE( Color_t, colLocalViewModelChamsColor, Color_t( 210, 134, 240 ) );
	C_ADD_VARIABLE( int, eLocalWeaponChams, kMaterialDisabled );
	C_ADD_VARIABLE( Color_t, colLocalWeaponChamsColor, Color_t( 210, 134, 240 ) );
#pragma endregion


#pragma region cheat_settings
	C_ADD_VARIABLE( Color_t, colUIBackgroundColor, Color_t( 31, 31, 31, 235) );
	C_ADD_VARIABLE( Color_t, colUIMainColor, Color_t( 132, 133, 255, static_cast< int >( 255 * .5f ) ) );
	C_ADD_VARIABLE( Color_t, colUILogoColor, Color_t( 255, 255, 255 ) );
	C_ADD_VARIABLE( Color_t, colUITextColor, Color_t( 255, 255, 255 ) );
	C_ADD_VARIABLE( Color_t, colUIWatermarkTextColor, Color_t( 132, 133, 255 ) );
	C_ADD_VARIABLE( Color_t, colUIWatermarkBackgroundColor, Color_t( 31, 31, 31, 255 ) );
	C_ADD_VARIABLE( Color_t, colUICursorColor, Color_t( 66, 237, 146, 200 ) );

	C_ADD_VARIABLE( int, iDpiScale, 1 );

	C_ADD_VARIABLE( unsigned int, nCheatUnloadKey, VK_END );
	C_ADD_VARIABLE( unsigned int, nOpenMenuKey, VK_INSERT );
#pragma endregion
};

inline Variables_t Vars = {};