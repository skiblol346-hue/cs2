#pragma once
#include <SafetyHook/safetyhook.hpp>
#include <Core/Sdk/EntityData.hpp>

inline SafetyHookInline OnAddEntity_o{};
auto __fastcall Hook_OnAddEntity( void* rcx, CEntityInstance* pInstance, CBaseHandle hHandle )->void*;
