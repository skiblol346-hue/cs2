#pragma once

class CConVar;

namespace CONVAR
{
	bool Setup( );

	inline CConVar* game_type = nullptr;
	inline CConVar* game_mode = nullptr;

	inline CConVar* mp_teammates_are_enemies = nullptr;

	inline CConVar* viewmodel_offset_x = nullptr;
	inline CConVar* viewmodel_offset_y = nullptr;
	inline CConVar* viewmodel_offset_z = nullptr;
	inline CConVar* viewmodel_fov = nullptr;

	inline CConVar* r_fullscreen_gamma = nullptr;

	inline CConVar* sc_no_vis = nullptr;

	inline CConVar* name = nullptr;

	inline CConVar* sv_maxunlag = nullptr;
}
