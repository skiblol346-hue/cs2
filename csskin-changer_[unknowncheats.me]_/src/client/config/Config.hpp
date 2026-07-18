#pragma once
#include <cstdint>
#include <memory>

#ifdef _DEBUG
    #define CONSOLE_ENABLED
#endif

class Config {
public:
	struct knife_changer_t {
		bool m_enabled = false;
		int m_knife = 0;
		int m_paint_kit = 0;
		float m_wear = 0.0001f;
		int m_seed = 0;
		bool m_stattrak_enabled = false;
		int m_stattrak_count = 0;
		char m_custom_name[161] = {};
		bool m_custom_colors_enabled = false;
		float m_custom_colors[4][4] = {
			{ 1.0f, 1.0f, 1.0f, 1.0f },
			{ 1.0f, 1.0f, 1.0f, 1.0f },
			{ 1.0f, 1.0f, 1.0f, 1.0f },
			{ 1.0f, 1.0f, 1.0f, 1.0f },
		};
	} knife_changer;

	struct glove_changer_t {
		bool m_enabled = false;
		int m_glove = 0;
		int m_paint_kit = 0;
		float m_wear = 0.0001f;
		int m_seed = 0;
	} glove_changer;

	struct skin_changer_t {
		static constexpr int sticker_slot_count = 5;

		bool m_enabled = false;
		int m_selected_weapon = 0;
		bool weapon_cat_open[7] = { true, true, true, true, true, true, true };

		struct sticker_slot_t {
			int id = 0;
			float wear = 0.0f;
			float scale = 1.0f;
			float rotation = 0.0f;
			float offset_x = 0.0f;
			float offset_y = 0.0f;
		};

		struct keychain_t {
			int id = 0;
			float offset_x = 0.0f;
			float offset_y = 0.0f;
			float offset_z = 0.0f;
			int seed = 0;
		};

		struct weapon_skin_t {
			int paint_kit = 0;
			float wear = 0.0001f;
			int seed = 0;
			bool stattrak_enabled = false;
			int stattrak_count = 0;
			char custom_name[161] = {};
			bool custom_colors_enabled = false;
			float custom_colors[4][4] = {
				{ 1.0f, 1.0f, 1.0f, 1.0f },
				{ 1.0f, 1.0f, 1.0f, 1.0f },
				{ 1.0f, 1.0f, 1.0f, 1.0f },
				{ 1.0f, 1.0f, 1.0f, 1.0f },
			};
			sticker_slot_t stickers[sticker_slot_count];
			keychain_t keychain;
		};

		weapon_skin_t weapon_skins[100];

		static int get_config_index(uint16_t def_index) {
			if (def_index >= 1 && def_index <= 70) return def_index;
			return 0;
		}
	} skin_changer;
	struct agent_changer_t {
		bool m_enabled = false;
		int m_agent_ct = 0;
		int m_agent_t = 0;
	} agent_changer;
	struct misc_t
	{
		bool m_quick_stop = false;
		bool m_auto_accept = false;
		bool m_auto_accept_status_text = true;
		bool m_spectator_list = false;
		float m_spectator_list_x = 20.0f;
		float m_spectator_list_y = 220.0f;
		int m_menu_key = 0x2E;
	} misc;

};
