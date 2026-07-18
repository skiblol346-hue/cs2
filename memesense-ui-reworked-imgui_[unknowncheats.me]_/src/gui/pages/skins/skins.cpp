#include "skins.hpp"
#include "../../framework/shell/shell.hpp"
#include "../../framework/styling.hpp"
#include "imgui.h"

#include <cstdio>

namespace vars
{
	bool playerModel = false;
	bool gloves = true;
	int gloveType = 0;
	int gloveSkin = 0;
	int gloveQuality = 100;
	int gloveSeed = 0;
	bool knifeEnabled = true;
	int knife = 0;
	bool modelPhysics = false;
	bool applyPickup = false;
	int shared = 0;
	bool weaponEnabled = false;
	bool stattrak = false;
	int wpnQuality = 0;
	int wpnSeed = 0;
	bool overrideColors = false;
	bool bothTeams = false;
	int wpnList = 0;
	float skinColors[4][4] = {
		{ 0.85f, 0.27f, 0.27f, 1.f }, { 0.40f, 0.62f, 0.86f, 1.f },
		{ 0.55f, 0.85f, 0.20f, 1.f }, { 0.85f, 0.78f, 0.25f, 1.f }
	};
}

namespace pages
{
	void RenderSkins() noexcept
	{
		static const char* kGloveTypes[] = {
			"Sport Gloves", "Driver Gloves", "Hand Wraps", "Moto Gloves",
			"Specialist Gloves", "Bloodhound Gloves", "Hydra Gloves"
		};
		static const char* kGloveSkins[] = {
			"Superconductor", "Arid", "Pandora's Box", "Vice", "Omega", "Slingshot", "Nocts"
		};
		static const char* kKnives[] = {
			"Karambit", "Bayonet", "M9 Bayonet", "Butterfly Knife", "Flip Knife",
			"Gut Knife", "Huntsman Knife", "Falchion Knife", "Talon Knife", "Skeleton Knife"
		};
		static const char* kSharedMode[] = { "Off", "Teammates", "Everyone" };

		if (ksd::BeginChild("Model customization", { 0, 0, 3, 6 }))
		{
			ksd::Checkbox("Player model", &vars::playerModel);

			ksd::Checkbox("Gloves", &vars::gloves);
			if (vars::gloves)
			{
				ksd::SingleSelect("", &vars::gloveType, kGloveTypes, 7);
				ksd::SingleSelect("", &vars::gloveSkin, kGloveSkins, 7);
				ksd::SliderInt("Quality", &vars::gloveQuality, 0, 100, "%d%%");
				ksd::SliderInt("Seed", &vars::gloveSeed, 0, 1000, "%d");
			}

			ksd::Checkbox("Knife", &vars::knifeEnabled);
			if (vars::knifeEnabled)
				ksd::SingleSelect("", &vars::knife, kKnives, 10);

			ksd::Checkbox("Enable model physics", &vars::modelPhysics);
		}
		ksd::EndChild();

		if (ksd::BeginChild("Settings", { 0, 8, 3, 2 }))
		{
			ksd::Checkbox("Apply skins to picked up weapons", &vars::applyPickup);
			ksd::SingleSelect("Shared skin changer", &vars::shared, kSharedMode, 3);
		}
		ksd::EndChild();

		static const char* kSkinList[] = {
			"AK-47", "M4A4 / M4A1-S", "AWP", "Desert Eagle", "USP-S / Glock-18",
			"SSG 08", "SG 553", "AUG", "P250", "Five-SeveN"
		};

		if (ksd::BeginChild("Weapon skin", { 6, 0, 3, 10 }))
		{
			ksd::Checkbox("Enabled", &vars::weaponEnabled);
			ksd::Checkbox("StatTrak\xE2\x84\xA2", &vars::stattrak);
			ksd::SliderInt("Quality", &vars::wpnQuality, 0, 100, "%d%%");
			ksd::SliderInt("Seed", &vars::wpnSeed, 0, 1000, "%d");

			{
				const ImVec2 row    = ksd::CursorPos();
				const float  rightX = ksd::PanelRightX();
				ksd::Checkbox("Override skin colors", &vars::overrideColors);
				for (int i = 0; i < 4; ++i)
				{
					char id[24];
					std::snprintf(id, sizeof(id), "##skin_col%d", i);
					ksd::ColorButton(id, vars::skinColors[3 - i],
						ImVec2(rightX - 17.f - i * 20.f, row.y + 2.f), ImVec2(16.f, 10.f));
				}
			}

			ksd::Checkbox("Apply to both teams", &vars::bothTeams);

			static char search[64] = {};
			ksd::InputText("##skin_search", search, sizeof(search));

			ksd::ListBox("##skin_list", &vars::wpnList, kSkinList, 10);

			static char nameTag[64] = {};
			ksd::InputText("##skin_nametag", nameTag, sizeof(nameTag));
		}
		ksd::EndChild();
	}
}
