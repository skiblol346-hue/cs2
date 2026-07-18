#pragma once

#include <XorStr/XorStr.hpp>
#define JM_XORSTR_DISABLE_AVX_INTRINSICS
#define _xor(STRING) xorstr_(STRING)

#define CS_VERSION 10902
#define CS_CONFIGURATION_BINARY

#define CHEAT_NAME                          _xor("Stellarix")
#define CONSOLE_NAME                        _xor(L"Stellarix-Console")
#define CHEAT_VERSION                       _xor("1.0.0")

#define CONFIG_DIRECTORY                    _xor("configs")
#define L_CONFIG_DIRECTORY                    _xor("configs")
#define HITSOUND_DIRECTORY                  _xor("hitsounds")
#define DEFAULT_CONFIG                      _xor("default")
#define L_DEFAULT_CONFIG                    L"default"
#define CONFIG_EXTENSION                    _xor(".cfg")
#define L_CONFIG_EXTENSION                  L".cfg"
#define LOG_FILENAME                        _xor("log.txt")


#ifdef _DEBUG
#define ENABLE_CONSOLE_DEBUG                1
#else
#define ENABLE_CONSOLE_DEBUG                0
#endif

#define ENABLE_RAGE                         1



#define CS_COMPILER_MSC

#ifndef CS_DEBUG_BREAK
#if defined(CS_COMPILER_MSC)
#define CS_DEBUG_BREAK() __debugbreak()
#elif defined(CS_COMPILER_CLANG)
#define CS_DEBUG_BREAK() __builtin_debugtrap()
#else
static_assert( false, "it is expected you to define CS_DEBUG_BREAK() into something that will break in a debugger!" );
#define CS_DEBUG_BREAK()
#endif
#endif


#ifndef CS_ASSERT
#ifdef _DEBUG
#define CS_ASSERT(EXPRESSION) static_cast<void>(!!(EXPRESSION) || (CS_DEBUG_BREAK(), 0))
#else
// disable assertion for release builds
#define CS_ASSERT(EXPRESSION) static_cast<void>(0)
#endif
#endif



#define _CS_INTERNAL_CONCATENATE(LEFT, RIGHT) LEFT##RIGHT
#define _CS_INTERNAL_STRINGIFY(NAME) #NAME
#define _CS_INTERNAL_UNPARENTHESIZE(...) __VA_ARGS__

// convert plain text to string
#define CS_STRINGIFY(NAME)              _CS_INTERNAL_STRINGIFY(NAME)
// concatenate plain text
#define CS_CONCATENATE(LEFT, RIGHT)     _CS_INTERNAL_CONCATENATE(LEFT, RIGHT)
// unparenthesize variadic arguments
#define CS_UNPARENTHESIZE(...)          _CS_INTERNAL_UNPARENTHESIZE(__VA_ARGS__)

// calculate elements count of fixed-size CONFIG array
#define CS_ARRAYSIZE(ARRAY)             (sizeof(ARRAY) / sizeof(ARRAY[0]))


#define CS_CLASS_NO_CONSTRUCTOR(CLASS) \
	CLASS() = delete;                  \
	CLASS(CLASS&&) = delete;           \
	CLASS(const CLASS&) = delete;

/*
	* explicitly delete the following assignment operators, to prevent attempts on using them:
	* move-assignment, copy-assignment
	*/
#define CS_CLASS_NO_ASSIGNMENT(CLASS)   \
	CLASS& operator=(CLASS&&) = delete; \
	CLASS& operator=(const CLASS&) = delete;

	// explicitly delete any class initializer to prevent attempts on using them
#define CS_CLASS_NO_INITIALIZER(CLASS) \
	CS_CLASS_NO_CONSTRUCTOR(CLASS)     \
	CS_CLASS_NO_ASSIGNMENT(CLASS)

// explicitly delete class heap allocator and deallocator, to prevent attempts on using class at heap memory
#define CS_CLASS_NO_ALLOC()                               \
	void* operator new(const std::size_t nSize) = delete; \
	void operator delete(void* pMemory) = delete;




#define CLIENT_DLL                          _xor(L"client.dll")
#define ENGINE2_DLL                         _xor(L"engine2.dll")
#define SCHEMASYSTEM_DLL                    _xor(L"schemasystem.dll")
#define INPUTSYSTEM_DLL                     _xor(L"inputsystem.dll")
#define SDL3_DLL                            _xor(L"SDL3.dll")
#define TIER0_DLL                           _xor(L"tier0.dll")
#define NAVSYSTEM_DLL                       _xor(L"navsystem.dll")
#define RENDERSYSTEM_DLL                    _xor(L"rendersystemdx11.dll")
#define LOCALIZE_DLL                        _xor(L"localize.dll")
#define DBGHELP_DLL                         _xor(L"dbghelp.dll")
#define GAMEOVERLAYRENDER64_DLL             _xor(L"gameoverlayrenderer64.dll")
#define PARTICLES_DLL                       _xor(L"particles.dll")
#define SCENESYSTEM_DLL                     _xor(L"scenesystem.dll")
#define MATERIAL_SYSTEM2_DLL                _xor(L"materialsystem2.dll")
#define MATCHMAKING_DLL                     _xor(L"matchmaking.dll")
#define RESOURCESYSTEM_DLL                  _xor(L"resourcesystem.dll")
#define SOUNDSYSTEM_DLL                     _xor(L"soundsystem.dll")
#define ANIMATIONSYSTEM_DLL                 _xor(L"animationsystem.dll")





#pragma region sdk_definitons
#define GAME_RESOURCE_SERVICE_CLIENT        _xor("GameResourceServiceClientV001")
#define SOURCE2_CLIENT                      _xor("Source2Client002")
#define SCHEMA_SYSTEM                       _xor("SchemaSystem_001")
#define INPUT_SYSTEM_VERSION                _xor("InputSystemVersion001")
#define SOURCE2_ENGINE_TO_CLIENT            _xor("Source2EngineToClient001")
#define ENGINE_CVAR                         _xor("VEngineCvar007")
#define LOCALIZE                            _xor("Localize_001")
#define NETWORK_CLIENT_SERVICE              _xor("NetworkClientService_001")
#define MATERIAL_SYSTEM2                    _xor("VMaterialSystem2_001")
#define RESOURCE_SYSTEM                     _xor("ResourceSystem013")
#define RESOURCE_HANDLE_UTILS               _xor("ResourceHandleUtils001")
#define C_PREDICTION                        _xor("Source2ClientPrediction001")
#define PARTICLE_SYSTEM_MANAGER             _xor("ParticleSystemMgr00")
#pragma endregion



// @source: master/game/shared/shareddefs.h
#define TICK_INTERVAL 0.015625f
#define TIME_TO_TICKS(TIME) (static_cast<int>(0.5f + static_cast<float>(TIME) / TICK_INTERVAL))
#define TICKS_TO_TIME(TICKS) (TICK_INTERVAL * static_cast<float>(TICKS))
#define ROUND_TO_TICKS(TIME) (TICK_INTERVAL * TIME_TO_TICKS(TIME))
#define TICK_NEVER_THINK (-1)
#pragma endregion