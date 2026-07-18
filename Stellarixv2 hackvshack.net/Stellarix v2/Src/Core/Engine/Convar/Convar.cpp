#include "Convar.hpp"
#include <Core/Interfaces/InterfaceManager.hpp>
#include <Core/Interfaces/List/IEngineCVar.hpp>


bool CONVAR::Setup( )
{
	bool bSuccess = true;

	game_type = GetInterfaceManager( )->GetEngineCVar( )->Find( FNV1A::HashConst( "game_type" ) );
	bSuccess &= game_type != nullptr;

	game_mode = GetInterfaceManager( )->GetEngineCVar( )->Find( FNV1A::HashConst( "game_mode" ) );
	bSuccess &= game_mode != nullptr;

	mp_teammates_are_enemies = GetInterfaceManager( )->GetEngineCVar( )->Find( FNV1A::HashConst( "mp_teammates_are_enemies" ) );
	bSuccess &= mp_teammates_are_enemies != nullptr;


	viewmodel_offset_x = GetInterfaceManager( )->GetEngineCVar( )->Find( FNV1A::HashConst( "viewmodel_offset_x" ) );
	bSuccess &= viewmodel_offset_x != nullptr;

	viewmodel_offset_y = GetInterfaceManager( )->GetEngineCVar( )->Find( FNV1A::HashConst( "viewmodel_offset_y" ) );
	bSuccess &= viewmodel_offset_y != nullptr;

	viewmodel_offset_z = GetInterfaceManager( )->GetEngineCVar( )->Find( FNV1A::HashConst( "viewmodel_offset_z" ) );
	bSuccess &= viewmodel_offset_z != nullptr;
	
	viewmodel_fov = GetInterfaceManager( )->GetEngineCVar( )->Find( FNV1A::HashConst( "viewmodel_fov" ) );
	bSuccess &= viewmodel_fov != nullptr;
	
	
	r_fullscreen_gamma = GetInterfaceManager( )->GetEngineCVar( )->Find( FNV1A::HashConst( "r_fullscreen_gamma" ) );
	bSuccess &= r_fullscreen_gamma != nullptr;


	sc_no_vis = GetInterfaceManager( )->GetEngineCVar( )->Find( FNV1A::HashConst( "sc_no_vis" ) );
	bSuccess &= sc_no_vis != nullptr;
	
	
	name = GetInterfaceManager( )->GetEngineCVar( )->Find( FNV1A::HashConst( "name" ) );
	bSuccess &= name != nullptr;
	
	
	sv_maxunlag = GetInterfaceManager( )->GetEngineCVar( )->Find( FNV1A::HashConst( "sv_maxunlag" ) );
	bSuccess &= sv_maxunlag != nullptr;

	return bSuccess;
}
