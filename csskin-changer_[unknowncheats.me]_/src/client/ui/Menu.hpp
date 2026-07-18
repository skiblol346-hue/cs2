#pragma once

enum class menu_tab_t {
	skins = 0,
	setting = 1
};

enum class skins_sidebar_t {
	general = 0,
	weapons = 1,
	knives = 2,
	gloves = 3,
	agent = 4
};

enum class setting_sidebar_t {
	config = 0,
	misc = 1
};

class Menu {
	int m_selected_tab = 0;
	int m_selected_skins_tab = 1; // default to weapons
	int m_selected_setting_tab = 0; // default to config
	float m_anim = 0.0f;
	bool m_style_initialized = false;
	bool m_waiting_for_key = false;
	float m_dpi_scale = 1.0f;

public:
	bool m_opened = true;

	void draw();
	void on_create_move();
	void rebuild_fonts(float scale);
	void ensure_style();

	bool is_open() const { return m_opened; }
	bool is_waiting_for_key() const { return m_waiting_for_key; }
	void set_waiting_for_key(bool waiting) { m_waiting_for_key = waiting; }
	float get_dpi_scale() const { return m_dpi_scale; }
	void set_dpi_scale(float scale) { m_dpi_scale = scale; }
};
