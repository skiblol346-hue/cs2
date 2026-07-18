#pragma once

#include "imgui.h"

struct nl_color_t {
	float r;
	float g;
	float b;
	float a;

	ImColor to_im_color(float alpha = 1.0f, bool use_style_alpha = true) const;
	ImVec4 to_vec4(float alpha = 1.0f, bool use_style_alpha = true) const;
};

class MenuGui {
public:
	float m_anim = 0.0f;

	nl_color_t accent_color = { 0.00f, 1.00f, 0.20f, 1.00f };
	nl_color_t text = { 1.00f, 1.00f, 1.00f, 1.00f };
	nl_color_t text_disabled = { 0.50f, 0.50f, 0.50f, 1.00f };
	nl_color_t border = { 0.12f, 0.12f, 0.12f, 1.00f };
	nl_color_t frame_inactive = { 0.06f, 0.06f, 0.06f, 1.00f };
	nl_color_t frame_active = { 0.10f, 0.10f, 0.10f, 1.00f };
	nl_color_t button = { 0.05f, 0.05f, 0.05f, 1.00f };
	nl_color_t button_hovered = { 0.12f, 0.12f, 0.12f, 1.00f };
	nl_color_t button_active = { 0.16f, 0.16f, 0.16f, 1.00f };
	nl_color_t group_box_bg = { 0.04f, 0.04f, 0.04f, 1.00f };

	void group_title(const char* name);
	void group_box(const char* name, ImVec2 size_arg);
	void end_group_box();

	bool tab(const char* icon, const char* label, bool selected);
	bool subtab(const char* label, bool selected, int count, ImDrawFlags flags);
};

extern MenuGui g_menu_gui;
