#include "InputManager.hpp"
#include <StellarixUI.hpp>
#include <Core/Config/Variables.hpp>

static CInputManager g_CInputManager{ };

auto CInputManager::OnInit( ) -> bool
{
    return true;
}

auto CInputManager::OnDestroy( ) -> void
{
}

auto CInputManager::OnWindowProcess( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam ) -> bool
{
	// prevent process when e.g. binding something in-menu
	if ( wParam != C_GET( unsigned int, Vars.nOpenMenuKey ) && wParam != C_GET( unsigned int, Vars.nCheatUnloadKey ) && GetStellarixUI( )->bMainWindowOpened )
		return false;

	// current active key
	int nKey = 0;
	// current active key state
	KeyState_t state = KEY_STATE_NONE;

	switch ( uMsg )
	{
	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
		if ( wParam < 256U )
		{
			nKey = static_cast< int >( wParam );
			state = KEY_STATE_DOWN;
		}
		break;
	case WM_KEYUP:
	case WM_SYSKEYUP:
		if ( wParam < 256U )
		{
			nKey = static_cast< int >( wParam );
			state = KEY_STATE_UP;
		}
		break;
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_LBUTTONDBLCLK:
		nKey = VK_LBUTTON;
		state = uMsg == WM_LBUTTONUP ? KEY_STATE_UP : KEY_STATE_DOWN;
		break;
	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
	case WM_RBUTTONDBLCLK:
		nKey = VK_RBUTTON;
		state = uMsg == WM_RBUTTONUP ? KEY_STATE_UP : KEY_STATE_DOWN;
		break;
	case WM_MBUTTONDOWN:
	case WM_MBUTTONUP:
	case WM_MBUTTONDBLCLK:
		nKey = VK_MBUTTON;
		state = uMsg == WM_MBUTTONUP ? KEY_STATE_UP : KEY_STATE_DOWN;
		break;
	case WM_XBUTTONDOWN:
	case WM_XBUTTONUP:
	case WM_XBUTTONDBLCLK:
		nKey = ( GET_XBUTTON_WPARAM( wParam ) == XBUTTON1 ? VK_XBUTTON1 : VK_XBUTTON2 );
		state = uMsg == WM_XBUTTONUP ? KEY_STATE_UP : KEY_STATE_DOWN;
		break;
	default:
		return false;
	}

	// save key states
	if ( state == KEY_STATE_UP && arrKeyState[ nKey ] == KEY_STATE_DOWN ) // if swap states it will be pressed state
		arrKeyState[ nKey ] = KEY_STATE_RELEASED;
	else
		arrKeyState[ nKey ] = state;

	return true;
}

auto CInputManager::IsKeyReleased( const std::uint32_t uButtonCode ) -> bool
{
	if ( arrKeyState[ uButtonCode ] == KEY_STATE_RELEASED )
	{
		arrKeyState[ uButtonCode ] = KEY_STATE_UP;
		return true;
	}

	return false;
}

auto CInputManager::GetBindState( KeyBind_t& keyBind ) -> bool
{
	if ( keyBind.uKey == 0U )
		return false;

	switch ( keyBind.nMode )
	{
	case EKeyBindMode::HOLD:
		keyBind.bEnable = IsKeyDown( keyBind.uKey );
		break;
	case EKeyBindMode::TOGGLE:
		if ( IsKeyReleased( keyBind.uKey ) )
			keyBind.bEnable = !keyBind.bEnable;
		break;
	}

	return keyBind.bEnable;
}

auto CInputManager::GetBindState( std::vector<KeyBind_t>& keyBinds ) -> bool
{
	for ( KeyBind_t& bind : keyBinds )
	{
		if ( GetBindState( bind ) )
			return true;
	}

	return false;
}

auto CInputManager::GetBindState( std::vector<KeyBind_t>& keyBinds, KeyBind_t& keyBindActive ) -> bool
{
	for ( KeyBind_t& bind : keyBinds )
	{
		if ( GetBindState( bind ) )
		{
			keyBindActive = bind;
			return true;
		}
	}

	return false;
}

auto GetInputManager( ) -> CInputManager*
{
	return &g_CInputManager;
}
