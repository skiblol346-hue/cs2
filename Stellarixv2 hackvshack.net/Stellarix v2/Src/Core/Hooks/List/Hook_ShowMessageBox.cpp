#include "Hook_ShowMessageBox.hpp"

auto Hook_ShowMessageBox( void* pLegacyUiInterface, const char* szTitle, const char* szMessage, bool bShowOk, bool bShowCancel, const char* szCommandOk, const char* szCommandCancel, const char* szCommandClosed ) -> void
{
	ShowMessageBox_o.call<void>( pLegacyUiInterface, szTitle, szMessage, bShowOk, bShowCancel, szCommandOk, szCommandCancel, szCommandClosed );
}
