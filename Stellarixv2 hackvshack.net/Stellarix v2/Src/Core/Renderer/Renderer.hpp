#pragma once
#include <Core/Common.hpp>
#include <Core/Sdk/Datatypes/Vector.hpp>
#include <d3d11.h>
#include <Imgui/imgui.h>
#include <Imgui/imgui_internal.h>
#include <map>
#include <string>


namespace StellarixFonts
{
	inline ImFont* fontVerdana = nullptr;
	inline ImFont* fontInterBOLD = nullptr;
	inline ImFont* fontWeaponIcons = nullptr;
	inline ImFont* fontAwesome5 = nullptr;

	inline const char* GunWeaponIcon( const std::string& weapon )
	{
		static std::map<std::string, const char*> gunIcons = {
			{ _xor( "weapon_p90" ), "P" },
			{ _xor( "weapon_mp9" ), "O" },
			{ _xor( "weapon_mp5sd" ), "O" },
			{ _xor( "weapon_m4a4" ), "M" },
			{ _xor( "weapon_knife" ), "]" },
			{ _xor( "weapon_knife_ct" ), "]" },
			{ _xor( "weapon_knife_t" ), "]" },
			{ _xor( "weapon_deagle" ), "A" },
			{ _xor( "weapon_elite" ), "B" },
			{ _xor( "weapon_fiveseven" ), "C" },
			{ _xor( "weapon_glock" ), "D" },
			{ _xor( "weapon_revolver" ), "J" },
			{ _xor( "weapon_hkp2000" ), "E" },
			{ _xor( "weapon_p250" ), "F" },
			{ _xor( "weapon_usp_silencer" ), "G" },
			{ _xor( "weapon_tec9" ), "H" },
			{ _xor( "weapon_cz75a" ), "I" },
			{ _xor( "weapon_mac10" ), "K" },
			{ _xor( "weapon_ump45" ), "" },
			{ _xor( "weapon_bizon" ), "M" },
			{ _xor( "weapon_mp7" ), "N" },
			{ _xor( "weapon_galilar" ), "Q" },
			{ _xor( "weapon_famas" ), "R" },
			{ _xor( "weapon_m4a1_silencer" ), "T" },
			{ _xor( "weapon_m4a1" ), "S" },
			{ _xor( "weapon_aug" ), "U" },
			{ _xor( "weapon_sg556" ), "V" },
			{ _xor( "weapon_ak47" ), "W" },
			{ _xor( "weapon_g3sg1" ), "X" },
			{ _xor( "weapon_scar20" ), "Y" },
			{ _xor( "weapon_awp" ), "Z" },
			{ _xor( "weapon_ssg08" ), "a" },
			{ _xor( "weapon_xm1014" ), "b" },
			{ _xor( "weapon_sawedoff" ), "c" },
			{ _xor( "weapon_mag7" ), "d" },
			{ _xor( "weapon_nova" ), "e" },
			{ _xor( "weapon_negev" ), "f" },
			{ _xor( "weapon_m249" ), "g" },
			{ _xor( "weapon_taser" ), "h" },
			{ _xor( "weapon_flashbang" ), "i" },
			{ _xor( "weapon_hegrenade" ), "j" },
			{ _xor( "weapon_smokegrenade" ), "k" },
			{ _xor( "weapon_molotov" ), "l" },
			{ _xor( "weapon_decoy" ), "m" },
			{ _xor( "weapon_incgrenade" ), "n" },
			{ _xor( "weapon_c4" ), "o" },
			{ _xor( "weapon_defuse_kit" ), "r" },
			{ _xor( "weapon_bayonet" ), "]" },
			{ _xor( "weapon_knife_survival_bowie" ), "]" },
			{ _xor( "weapon_knife_butterfly" ), "]" },
			{ _xor( "weapon_knife_canis" ), "]" },
			{ _xor( "weapon_knife_cord" ), "]" },
			{ _xor( "weapon_knife_css" ), "]" },
			{ _xor( "weapon_knife_falchion" ), "]" },
			{ _xor( "weapon_knife_flip" ), "]" },
			{ _xor( "weapon_knife_gut" ), "]" },
			{ _xor( "weapon_knife_karambit" ), "]" },
			{ _xor( "weapon_knife_twinbade" ), "]" },
			{ _xor( "weapon_knife_kukri" ), "]" },
			{ _xor( "weapon_knife_m9_bayonet" ), "]" },
			{ _xor( "weapon_knife_outdoor" ), "]" },
			{ _xor( "weapon_knife_push" ), "]" },
			{ _xor( "weapon_knife_skeleton" ), "]" },
			{ _xor( "weapon_knife_stiletto" ), "]" },
			{ _xor( "weapon_knife_tactical" ), "]" },
			{ _xor( "weapon_knife_widowmaker" ), "]" },
			{ _xor( "weapon_knife_ursus" ), "]" }
		};

		auto it = gunIcons.find( weapon );
		if ( it != gunIcons.end( ) )
			return it->second;

		return "";
	}
}

namespace StellarixShaders
{
	inline ID3D11ShaderResourceView* shaderWatermarkUser = nullptr;
	inline ID3D11ShaderResourceView* shaderUserTestCat = nullptr;
	inline ID3D11ShaderResourceView* shaderGhost = nullptr;
};


class CRenderer
{
public:
	CRenderer( ) = default;
	~CRenderer( ) = default;

public:
	auto OnInit( ) -> bool;
	auto OnDestroy( ) -> void;

public:
	auto RenderDrawData( ImDrawData* pDrawData ) -> void;
	auto NewFrame( ) -> void;
	auto Render( ) -> void;

public:
	auto InitFonts( ) -> bool;
	auto InitShaders( ) -> bool;

public:
	auto WorldToScreen( const Vector_t& vecOrigin, ImVec2* pVecScreen ) -> bool;

public:
	HWND hWindow = nullptr;
	WNDPROC pOldWndProc = nullptr;

public:
	// active draw data container used to store
	ImDrawList* pDrawListActive = nullptr;
	// safe draw data container
	ImDrawList* pDrawListSafe = nullptr;
	// actual draw data container used to render
	ImDrawList* pDrawListRender = nullptr;

private:
	bool m_bInitialized = false;
};

auto GetRenderer( ) -> CRenderer*;