#pragma once
#include "Datatypes/Matrix.hpp"
#include "Datatypes/QAngle.hpp"
#include "EntityHandle.hpp"
#include <atomic>
#include <chrono>
#include <Core/Utils/Logger/Logger.hpp>
#include <ImGui/imgui.h>
#include <Launcher.hpp>
#include <string>
#include <thread>


struct CheatData_t
{
	ViewMatrix_t viewMatrix;
	float flGVFrameTime;         // Global Vars
	float flGVCurrentTime;       // Global Vars
	bool bIsInGame;
	std::string szLocalPlayerName;

	int iFps;
	std::string szCurrentTime;
};

struct LocalPlayer_t
{
	Vector_t vecOrigin;
	Vector_t vecEyePosition;
	QAngle_t angViewAngle;

	int iHealth;
};

inline CheatData_t CheatData = {};
inline LocalPlayer_t LocalPlayerData = {};