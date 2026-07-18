#pragma once
#include <SafetyHook/safetyhook.hpp>

class CAggregateSceneObjectLighting;

inline SafetyHookInline RenderLighting_o{};
auto __fastcall Hook_RenderLighting( void* a1, CAggregateSceneObjectLighting* pCAggregateSceneObjectLighting, void* a2 ) -> void;