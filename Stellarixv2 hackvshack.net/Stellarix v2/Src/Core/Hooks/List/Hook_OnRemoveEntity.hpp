#pragma once
#include <SafetyHook/safetyhook.hpp>
#include <Core/Sdk/EntityData.hpp>

inline SafetyHookInline OnRemoveEntity_o{};
auto __fastcall Hook_OnRemoveEntity( void* rcx, CEntityInstance* pInstance, CBaseHandle hHandle )->void*;
