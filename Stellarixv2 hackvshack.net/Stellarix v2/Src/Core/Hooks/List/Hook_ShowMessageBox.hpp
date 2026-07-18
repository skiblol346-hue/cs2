#pragma once
#include <SafetyHook/safetyhook.hpp>


inline SafetyHookInline ShowMessageBox_o{};
auto Hook_ShowMessageBox( void* pLegacyUiInterface, const char* szTitle, const char* szMessage, bool bShowOk, bool bShowCancel, const char* szCommandOk, const char* szCommandCancel, const char* szCommandClosed ) -> void;
