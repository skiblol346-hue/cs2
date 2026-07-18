#pragma once
#include <Core/Memory/Memory.hpp>

class CAggregateSceneObjectLighting
{
private:
	MEM_PAD( 0xE4 ); // 0x0
public:
	float flRed;
	float flGreen;
	float flBlue;
};