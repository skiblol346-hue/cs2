#pragma once
#include <Core/Memory/Memory.hpp>


enum EPredictionReason : unsigned int
{
	client_command_tick,
	demo_preentity,
	demo_simulation,
	postnetupdate,
	server_starved_and_added_usercmds,
	client_frame_simulate
};

class CNetworkGameClient
{
private:
	MEM_PAD( 0xF0 );
public:
	bool bShouldPredict;
private:
	MEM_PAD( 0x14B );
public:
	int iDeltaTick;
private:
	MEM_PAD( 0x11C );
public:
	int iNonInterpolatedTick;

public:
	auto ClientSidePredict( EPredictionReason iPredReason ) -> void
	{
		using prediction_fn = void( __fastcall* )( CNetworkGameClient*, unsigned int );
		static prediction_fn func = reinterpret_cast< prediction_fn >( GetMemoryManager( )->FindPattern( ENGINE2_DLL, _xor( "40 55 41 56 48 83 EC ? 80 B9" ) ) );

		return func( this, iPredReason );
	}
};


class INetworkClientService
{
public:
	auto GetNetworkGameClient( ) -> CNetworkGameClient*
	{
		return GetMemoryManager( )->CallVFunc<CNetworkGameClient*, 23U>( this );
	}
};
