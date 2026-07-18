#pragma once
// used: [win] winapi
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <cstdint>
#include <vector>


struct KeyBind_t;

using KeyState_t = std::uint8_t;
enum EKeyState : KeyState_t
{
	KEY_STATE_NONE,
	KEY_STATE_DOWN,
	KEY_STATE_UP,
	KEY_STATE_RELEASED
};


class CInputManager
{
public:
	auto OnInit( ) -> bool;
	auto OnDestroy( ) -> void;

public:
	auto OnWindowProcess( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam ) -> bool;

public:
	auto IsKeyReleased( const std::uint32_t uButtonCode ) -> bool;
	auto GetBindState( KeyBind_t& keyBind ) -> bool;
	auto GetBindState( std::vector<KeyBind_t>& keyBinds ) -> bool;
	auto GetBindState( std::vector<KeyBind_t>& keyBinds, KeyBind_t& keyBindActive ) -> bool;
	auto IsKeyDown( const std::uint32_t uButtonCode ) -> bool
	{
		return arrKeyState[ uButtonCode ] == KEY_STATE_DOWN;
	}


public:
	KeyState_t arrKeyState[ 256 ] = {};
};


auto GetInputManager( ) -> CInputManager*;