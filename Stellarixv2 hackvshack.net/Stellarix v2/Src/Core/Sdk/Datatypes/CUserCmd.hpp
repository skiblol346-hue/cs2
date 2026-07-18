#pragma once
#include "../protobuf/cs_usercmd.pb.h"
#include <Core/Memory/Memory.hpp>
#include <Core/Sdk/EntityData.hpp>

// @source: server.dll
enum ECommandButtons : int
{
	IN_ATTACK = ( 1 << 0 ),
	IN_JUMP = ( 1 << 1 ),
	IN_DUCK = ( 1 << 2 ),
	IN_FORWARD = ( 1 << 3 ),
	IN_BACK = ( 1 << 4 ),
	IN_USE = ( 1 << 5 ),
	IN_CANCEL = ( 1 << 6 ),
	IN_LEFT = ( 1 << 7 ),
	IN_RIGHT = ( 1 << 8 ),
	IN_MOVELEFT = ( 1 << 9 ),
	IN_MOVERIGHT = ( 1 << 10 ),
	IN_SECOND_ATTACK = ( 1 << 11 ),
	IN_RUN = ( 1 << 12 ),
	IN_RELOAD = ( 1 << 13 ),
	IN_LEFT_ALT = ( 1 << 14 ),
	IN_RIGHT_ALT = ( 1 << 15 ),
	IN_SCORE = ( 1 << 16 ),
	IN_SPEED = ( 1 << 17 ),
	IN_WALK = ( 1 << 18 ),
	IN_ZOOM = ( 1 << 19 ),
	IN_FIRST_WEAPON = ( 1 << 20 ),
	IN_SECOND_WEAPON = ( 1 << 21 ),
	IN_BULLRUSH = ( 1 << 22 ),
	IN_FIRST_GRENADE = ( 1 << 23 ),
	IN_SECOND_GRENADE = ( 1 << 24 ),
	IN_MIDDLE_ATTACK = ( 1 << 25 ),
	IN_USE_OR_RELOAD = ( 1 << 26 )
};

class CInButtonState
{
	void* nVtable;
public:
	uint64_t nButtonstate1;
	uint64_t nButtonstate2;
	uint64_t nButtonstate3;

public:
    enum e_button_state_t : int8_t
    {
        in_button_up = 0,
        in_button_down = 1,
        in_button_down_up = 2,
        in_button_up_down = 3,
        in_button_up_down_up = 4,
        in_button_down_up_down = 5,
        in_button_down_up_down_up = 6,
        in_button_up_down_up_down = 7
    };

public:
    auto SetButtonState( const uint64_t& u_value, int e_button_state ) -> void
    {
        switch ( e_button_state )
        {
        case in_button_up:
        {
            nButtonstate1 &= ~u_value;
            nButtonstate2 &= ~u_value;
            nButtonstate3 &= ~u_value;
            break;
        }
        case in_button_down:
        {
            nButtonstate1 |= u_value;
            nButtonstate2 &= ~u_value;
            nButtonstate3 &= ~u_value;
            break;
        }
        case in_button_down_up:
        {
            nButtonstate1 &= ~u_value;
            nButtonstate2 |= u_value;
            nButtonstate3 &= ~u_value;
            break;
        }
        case in_button_up_down:
        {
            nButtonstate1 |= u_value;
            nButtonstate2 |= u_value;
            nButtonstate3 &= ~u_value;
            break;
        }
        case in_button_up_down_up:
        {
            nButtonstate1 &= ~u_value;
            nButtonstate2 &= ~u_value;
            nButtonstate3 |= u_value;
            break;
        }
        case in_button_down_up_down:
        {
            nButtonstate1 |= u_value;
            nButtonstate2 &= ~u_value;
            nButtonstate3 |= u_value;
            break;
        }
        case in_button_down_up_down_up:
        {
            nButtonstate1 &= ~u_value;
            nButtonstate2 |= u_value;
            nButtonstate3 |= u_value;
            break;
        }
        case in_button_up_down_up_down:
        {
            nButtonstate1 |= u_value;
            nButtonstate2 |= u_value;
            nButtonstate3 |= u_value;
            break;
        }
        }
    }
};


class CUserCmd
{
public:
	CUserCmd( ) = default;

private:
	MEM_PAD( 0x10 );
public:
	CSGOUserCmdPB csgoUserCmd;
	CInButtonState buttonState;
private:
	MEM_PAD( 0x10 );
public:
	bool bHasBeenPredicted;
private:
	MEM_PAD( 0xF );

public:
	static auto GetUserCmdBySequence( CCSPlayerController* pController ) -> CUserCmd*;
};