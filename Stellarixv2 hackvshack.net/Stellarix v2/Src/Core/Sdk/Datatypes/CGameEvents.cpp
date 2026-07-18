#include "CGameEvents.hpp"
#include <Core/Interfaces/InterfaceManager.hpp>
#include <Core/Interfaces/List/CGameEntitySystem.hpp>
#include <string>
#include <Core/StellarixClient.hpp>


static CEvents g_CEvents{ };

auto CGameEventHelper::GetPlayerController( ) -> CCSPlayerController*
{
	if ( !Event )
		return nullptr;

	int controller_id{};

	CBuffer buffer;
	buffer.name = "userid";

	Event->GetControllerId( controller_id, &buffer );

	if ( controller_id == -1 )
		return nullptr;

	return GetInterfaceManager( )->GetGameEntitySystem( )->Get<CCSPlayerController>( controller_id + 1 );
}

auto CGameEventHelper::GetAttackerController( ) -> CCSPlayerController*
{
	if ( !Event )
		return nullptr;

	int controller_id{};

	CBuffer buffer;
	buffer.name = "attacker";

	Event->GetControllerId( controller_id, &buffer );

	if ( controller_id == -1 )
		return nullptr;

	return GetInterfaceManager( )->GetGameEntitySystem( )->Get<CCSPlayerController>( controller_id + 1 );
}

auto CGameEventHelper::GetDamage( ) -> int
{
	if ( !Event )
		return -1;

	return Event->GetInt2( "dmg_health", false );
}

auto CGameEventHelper::GetBulletPos( ) -> Vector_t
{
	if ( !Event )
		return Vector_t( );

	std::string_view token_name_x = _xor( "x" );
	cUltStringToken tokenx( token_name_x.data( ) );

	std::string_view token_name_y = _xor( "y" );
	cUltStringToken tokeny( token_name_y.data( ) );

	std::string_view token_name_z = _xor( "z" );
	cUltStringToken tokenz( token_name_z.data( ) );

	return Vector_t( Event->GetFloatNew( tokenx ), Event->GetFloatNew( tokeny ), Event->GetFloatNew( tokenz ) );
}

auto CEvents::OnInit( ) -> bool
{
	GetInterfaceManager( )->GetGameEventManager( )->AddListener( this, _xor( "player_hurt" ), false );
	if ( !GetInterfaceManager( )->GetGameEventManager( )->FindListener( this, _xor( "player_hurt" ) ) )
		return false;

	GetInterfaceManager( )->GetGameEventManager( )->AddListener( this, _xor( "player_death" ), false );
	if ( !GetInterfaceManager( )->GetGameEventManager( )->FindListener( this, _xor( "player_death" ) ) )
		return false;

	GetInterfaceManager( )->GetGameEventManager( )->AddListener( this, _xor( "bullet_impact" ), false );
	if ( !GetInterfaceManager( )->GetGameEventManager( )->FindListener( this, _xor( "bullet_impact" ) ) )
		return false;

	GetInterfaceManager( )->GetGameEventManager( )->AddListener( this, _xor( "round_start" ), false );
	if ( !GetInterfaceManager( )->GetGameEventManager( )->FindListener( this, _xor( "round_start" ) ) )
		return false;

	GetInterfaceManager( )->GetGameEventManager( )->AddListener( this, _xor( "player_spawn" ), false );
	if ( !GetInterfaceManager( )->GetGameEventManager( )->FindListener( this, _xor( "player_spawn" ) ) )
		return false;

	GetInterfaceManager( )->GetGameEventManager( )->AddListener( this, _xor( "begin_new_match" ), false );
	if ( !GetInterfaceManager( )->GetGameEventManager( )->FindListener( this, _xor( "begin_new_match" ) ) )
		return false;

	GetInterfaceManager( )->GetGameEventManager( )->AddListener( this, _xor( "game_end" ), false );
	if ( !GetInterfaceManager( )->GetGameEventManager( )->FindListener( this, _xor( "game_end" ) ) )
		return false;

	return true;
}

auto CEvents::OnFireGameEventHook( CGameEvent* event ) -> void
{
	if ( !event )
		return;

	auto GameEventHelper = event->GetEventHelper( );
	auto EventType = GetEventType( event );

	switch ( EventType )
	{
	case kEventNone:
		return;
	case kEventHurt:
		GetStellarixClient( )->OnPlayerHurtEvent( GameEventHelper, event );
		break;
	case kEventBulletImpact:
		GetStellarixClient( )->OnBulletImpactEvent( GameEventHelper, event );
		break;
	case kEventPlayerDeath:
		GetStellarixClient( )->OnPlayerDeathEvent( GameEventHelper, event );
		break;
	case kEventRoundStart:
		break;
	case kEventPlayerSpawn:
		GetStellarixClient( )->OnPlayerSpawnEvent( GameEventHelper, event );
		break;
	case kEventBeginNewMatch:
		GetStellarixClient( )->OnPlayerBeginNewMatchEvent( GameEventHelper, event );
		break;
	case kEventGameEnd:
		GetStellarixClient( )->OnGameEndEvent( GameEventHelper, event );
		break;
	default:
		break;
	}
}

auto CEvents::GetEventType( CGameEvent* event ) -> EEventType
{
	FNV1A_t szEventName = FNV1A::Hash( event->GetName( ) );
	switch ( szEventName )
	{
	case FNV1A::HashConst( "player_hurt" ):
		return EEventType::kEventHurt;
	case FNV1A::HashConst( "player_death" ):
		return EEventType::kEventPlayerDeath;
	case FNV1A::HashConst( "bullet_impact" ):
		return EEventType::kEventBulletImpact;
	case FNV1A::HashConst( "round_start" ):
		return EEventType::kEventRoundStart;
	case FNV1A::HashConst( "player_spawn" ):
		return EEventType::kEventPlayerSpawn;
	case FNV1A::HashConst( "begin_new_match" ):
		return EEventType::kEventBeginNewMatch;
	case FNV1A::HashConst( "game_end" ):
		return EEventType::kEventGameEnd;
	}

	return EEventType::kEventNone;
}

auto GetGameEventListener( ) -> CEvents*
{
	return &g_CEvents;
}
