#pragma once
#include <windows.h>
#include <thread>


class CCheatLauncher final
{
public:
	CCheatLauncher( ) = default;
	~CCheatLauncher( ) = default;

public:
	auto OnCheatMain( HINSTANCE hInstance ) -> void;
	auto OnDestroy( ) -> void;

public:
	auto CheatIsAlive( ) -> bool
	{
		return m_bInitialized && !m_bDestroyed;
	}
	auto CheatHasBeenDestroyed( ) -> bool
	{
		return m_bDestroyed;
	}

private:
	bool m_bInitialized = false;
	bool m_bDestroyed = false;
};

auto GetCheatLauncher( ) -> CCheatLauncher*;