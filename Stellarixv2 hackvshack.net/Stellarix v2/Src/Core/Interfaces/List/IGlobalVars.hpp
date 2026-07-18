#pragma once
#include <cstdint>
#include <Core/Memory/Memory.hpp>

class IGlobalVars
{
public:
    float flRealTime;             // 0x0000
    int32_t nFrameCount;          // 0x0004
    float flFrameTime;            // 0x0008
    float flFrameTime2;           // 0x000C
    int32_t nMaxClients;          // 0x0010
    MEM_PAD( 0x1C );                  // 0x0014
    float flCurrentTime;          // 0x0030
    float flFrameTime3;           // 0x0034
    float flTickFraction;         // 0x0038
    float flTickFraction2;        // 0x003C
    MEM_PAD( 0x8 );                   // 0x0040
    int32_t nTickCount;           // 0x0048
    MEM_PAD( 0xC );                   // 0x004C
    //CNetChan* pNetChannel;        // 0x0058
    //MEM_PAD( 0x118 );                 // 0x0060
    //const char* szMapName;        // 0x0178
    //const char* szMapNameShort;   // 0x0180
};
