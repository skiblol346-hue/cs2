#include "platform/ClientCore.hpp"

#include "runtime/Runtime.hpp"
#include "ui/Menu.hpp"
#include "ui/MenuAssets.hpp"
#include "ui/MenuGui.hpp"
#include "ui/tabs/SettingsTab.hpp"
#include "ui/tabs/SkinsTab.hpp"
#include "imgui_impl_dx11.h"

#include <algorithm>
static bool merge_font_if_exists(const char* path, const ImWchar* ranges, float size) {
	if (GetFileAttributesA(path) == INVALID_FILE_ATTRIBUTES)
		return false;

	ImFontConfig config;
	config.MergeMode = true;
	config.PixelSnapH = true;
	config.OversampleH = 1;
	config.OversampleV = 1;

	return ImGui::GetIO().Fonts->AddFontFromFileTTF(path, size, &config, ranges) != nullptr;
}

static void merge_windows_text_fallbacks(float scale) {
	auto& io = ImGui::GetIO();
	const float size = 14.0f * scale;

	merge_font_if_exists("C:\\Windows\\Fonts\\segoeui.ttf", io.Fonts->GetGlyphRangesCyrillic(), size);
	merge_font_if_exists("C:\\Windows\\Fonts\\msyh.ttc", io.Fonts->GetGlyphRangesChineseFull(), size);
	if (!merge_font_if_exists("C:\\Windows\\Fonts\\YuGothR.ttc", io.Fonts->GetGlyphRangesJapanese(), size))
		merge_font_if_exists("C:\\Windows\\Fonts\\msgothic.ttc", io.Fonts->GetGlyphRangesJapanese(), size);
	merge_font_if_exists("C:\\Windows\\Fonts\\LeelawUI.ttf", io.Fonts->GetGlyphRangesThai(), size);
}

void Menu::rebuild_fonts(float scale) {
	auto& io = ImGui::GetIO();
	io.Fonts->Clear();

	ImFontConfig body_config;
	body_config.SizePixels = 14.0f * scale;
	body_config.OversampleH = 2;
	body_config.OversampleV = 1;
	body_config.PixelSnapH = true;
	body_config.FontDataOwnedByAtlas = false;

	ImFont* body_font = io.Fonts->AddFontFromMemoryTTF(
		museo500_binary,
		static_cast<int>(museo500_binary_size),
		14.0f * scale,
		&body_config
	);

	if (body_font) {
		merge_windows_text_fallbacks(scale);

		static const ImWchar icon_ranges[] = { ICON_MIN_FA, ICON_MAX_FA, 0 };
		ImFontConfig icon_config;
		icon_config.MergeMode = true;
		icon_config.PixelSnapH = true;
		icon_config.FontDataOwnedByAtlas = false;
		io.Fonts->AddFontFromMemoryTTF(
			font_awesome_binary,
			static_cast<int>(font_awesome_binary_size),
			13.0f * scale,
			&icon_config,
			icon_ranges
		);

		ImFontConfig title_config;
		title_config.SizePixels = 28.0f * scale;
		title_config.OversampleH = 2;
		title_config.OversampleV = 1;
		title_config.PixelSnapH = true;
		title_config.FontDataOwnedByAtlas = false;
		io.Fonts->AddFontFromMemoryTTF(
			museo900_binary,
			static_cast<int>(museo900_binary_size),
			28.0f * scale,
			&title_config
		);
	}
	else {
		io.Fonts->AddFontDefault(&body_config);
	}

	io.Fonts->Build();

	ImGui_ImplDX11_InvalidateDeviceObjects();
	ImGui_ImplDX11_CreateDeviceObjects();

	m_dpi_scale = scale;
	m_style_initialized = false;
}

void Menu::ensure_style() {
	if (m_style_initialized)
		return;

	ImGuiStyle& style = ImGui::GetStyle();
	ImVec4* colors = style.Colors;
	const float scale = m_dpi_scale;

	colors[ImGuiCol_Text] = ImVec4(0.95f, 0.95f, 0.95f, 1.00f);
	colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
	colors[ImGuiCol_WindowBg] = ImVec4(0.02f, 0.02f, 0.02f, 1.00f);
	colors[ImGuiCol_ChildBg] = ImVec4(0.04f, 0.04f, 0.04f, 0.00f);
	colors[ImGuiCol_PopupBg] = ImVec4(0.05f, 0.05f, 0.05f, 1.00f);
	colors[ImGuiCol_Border] = ImVec4(0.12f, 0.12f, 0.12f, 1.00f);
	colors[ImGuiCol_FrameBg] = ImVec4(0.06f, 0.06f, 0.06f, 1.00f);
	colors[ImGuiCol_FrameBgHovered] = ImVec4(0.12f, 0.12f, 0.12f, 1.00f);
	colors[ImGuiCol_FrameBgActive] = ImVec4(0.16f, 0.16f, 0.16f, 1.00f);
	colors[ImGuiCol_CheckMark] = ImVec4(0.00f, 1.00f, 0.20f, 1.00f);
	colors[ImGuiCol_SliderGrab] = ImVec4(0.00f, 1.00f, 0.20f, 0.85f);
	colors[ImGuiCol_SliderGrabActive] = ImVec4(0.00f, 1.00f, 0.20f, 1.00f);
	colors[ImGuiCol_Button] = ImVec4(0.05f, 0.05f, 0.05f, 1.00f);
	colors[ImGuiCol_ButtonHovered] = ImVec4(0.12f, 0.12f, 0.12f, 1.00f);
	colors[ImGuiCol_ButtonActive] = ImVec4(0.16f, 0.16f, 0.16f, 1.00f);
	colors[ImGuiCol_Header] = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
	colors[ImGuiCol_HeaderHovered] = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);
	colors[ImGuiCol_HeaderActive] = ImVec4(0.20f, 0.30f, 0.50f, 0.80f);
	colors[ImGuiCol_Separator] = ImVec4(0.12f, 0.12f, 0.12f, 1.00f);
	colors[ImGuiCol_NavHighlight] = ImVec4(0.00f, 1.00f, 0.20f, 1.00f);
	colors[ImGuiCol_ScrollbarBg] = ImVec4(0.04f, 0.04f, 0.04f, 1.00f);
	colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.12f, 0.12f, 0.12f, 1.00f);
	colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.18f, 0.18f, 0.18f, 1.00f);
	colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.00f, 1.00f, 0.20f, 0.85f);

	style.WindowPadding = ImVec2(0.0f, 0.0f);
	style.FramePadding = ImVec2(8.0f * scale, 5.0f * scale);
	style.ItemSpacing = ImVec2(8.0f * scale, 8.0f * scale);
	style.WindowRounding = 10.0f * scale;
	style.ChildRounding = 8.0f * scale;
	style.FrameRounding = 5.0f * scale;
	style.PopupRounding = 5.0f * scale;
	style.ScrollbarRounding = 5.0f * scale;
	style.WindowBorderSize = 0.0f;
	style.FrameBorderSize = 1.0f;

	m_style_initialized = true;
}

static void reset_tab_animation(float& anim, int& current, int next) {
	if (current == next)
		return;

	current = next;
	anim = 0.0f;
}

void Menu::draw() {
	if (!m_opened)
		return;

	ensure_style();

	const float scale = m_dpi_scale;
	const ImVec2 window_size(960.0f * scale, 560.0f * scale);
	const float content_x = 160.0f * scale;
	const float content_right_margin = 15.0f * scale;

	ImGui::SetNextWindowSize(window_size, ImGuiCond_Always);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	ImGui::Begin("ttapp", nullptr,
		ImGuiWindowFlags_NoDecoration |
		ImGuiWindowFlags_NoBackground |
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoCollapse);

	ImGuiIO& io = ImGui::GetIO();
	ImVec2 pos = ImGui::GetWindowPos();
	ImVec2 size = ImGui::GetWindowSize();
	const ImVec2 max_pos(
		std::max(0.0f, io.DisplaySize.x - size.x),
		std::max(0.0f, io.DisplaySize.y - size.y)
	);
	const ImVec2 clamped_pos(
		std::min(std::max(pos.x, 0.0f), max_pos.x),
		std::min(std::max(pos.y, 0.0f), max_pos.y)
	);

	if (clamped_pos.x != pos.x || clamped_pos.y != pos.y) {
		ImGui::SetWindowPos(clamped_pos, ImGuiCond_Always);
		pos = clamped_pos;
	}

	auto* draw = ImGui::GetWindowDrawList();

	// Background
	draw->AddRectFilled(pos, ImVec2(pos.x + size.x, pos.y + size.y), IM_COL32(8, 8, 8, 250), 10.0f * scale);

	// Top bar background
	draw->AddRectFilled(pos, ImVec2(pos.x + size.x, pos.y + 60.0f * scale), IM_COL32(0, 0, 0, 255), 10.0f * scale, ImDrawFlags_RoundCornersTop);

	// Top separator line
	draw->AddLine(ImVec2(pos.x, pos.y + 60.0f * scale), ImVec2(pos.x + size.x, pos.y + 60.0f * scale), IM_COL32(25, 25, 25, 255), 1.0f * scale);

	// Left sidebar background
	draw->AddRectFilled(ImVec2(pos.x, pos.y + 60.0f * scale), ImVec2(pos.x + 150.0f * scale, pos.y + size.y), IM_COL32(0, 0, 0, 255), 10.0f * scale, ImDrawFlags_RoundCornersBottomLeft);

	// Left sidebar separator line
	draw->AddLine(ImVec2(pos.x + 150.0f * scale, pos.y + 60.0f * scale), ImVec2(pos.x + 150.0f * scale, pos.y + size.y), IM_COL32(25, 25, 25, 255), 1.0f * scale);

	// Logo
	ImFont* title_font = io.Fonts->Fonts.Size > 1 ? io.Fonts->Fonts[1] : nullptr;
	if (title_font) {
		const char* part1 = "tt";
		const char* part2 = "app";
		const char* part3 = ".";
		
		ImVec2 part1_size = title_font->CalcTextSizeA(title_font->FontSize, FLT_MAX, 0.0f, part1);
		ImVec2 part2_size = title_font->CalcTextSizeA(title_font->FontSize, FLT_MAX, 0.0f, part2);
		
		float start_x = pos.x + 20.0f * scale;
		float start_y = pos.y + 30.0f * scale - part1_size.y * 0.5f;
		
		// Drop shadow
		draw->AddText(title_font, title_font->FontSize, ImVec2(start_x + 1.0f * scale, start_y + 1.0f * scale), IM_COL32(0, 0, 0, 180), part1);
		draw->AddText(title_font, title_font->FontSize, ImVec2(start_x + part1_size.x + 1.0f * scale, start_y + 1.0f * scale), IM_COL32(0, 0, 0, 180), part2);
		
		// tt in white
		draw->AddText(title_font, title_font->FontSize, ImVec2(start_x, start_y), IM_COL32(255, 255, 255, 255), part1);
		
		// app in neon green
		draw->AddText(title_font, title_font->FontSize, ImVec2(start_x + part1_size.x, start_y), g_menu_gui.accent_color.to_im_color(), part2);
		
		// dot in neon green
		draw->AddText(title_font, title_font->FontSize, ImVec2(start_x + part1_size.x + part2_size.x, start_y), g_menu_gui.accent_color.to_im_color(), part3);
	}
	else {
		draw->AddText(ImVec2(pos.x + 20.0f * scale, pos.y + 20.0f * scale), ImGui::GetColorU32(ImGuiCol_Text), "ttapp.");
	}

	// Avatar & Username at the bottom of left sidebar
	draw->AddCircleFilled(ImVec2(pos.x + 30.0f * scale, pos.y + size.y - 25.0f * scale), 15.0f * scale, IM_COL32(255, 255, 255, 255));
	draw->AddCircle(ImVec2(pos.x + 30.0f * scale, pos.y + size.y - 25.0f * scale), 15.0f * scale,
		g_menu_gui.accent_color.to_im_color(0.8f), 24, 1.0f * scale);
	draw->AddText(ImVec2(pos.x + 50.0f * scale, pos.y + size.y - 33.0f * scale), g_menu_gui.text.to_im_color(), "tana");

	// Top Navigation tabs
	ImGui::SetCursorPos(ImVec2(content_x + 10.0f * scale, 15.0f * scale));
	ImGui::BeginChild("##top_nav", ImVec2(400.0f * scale, 45.0f * scale), false, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoBackground);
	static const char* top_tabs[] = { "SKINS", "SETTING" };

	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0, 0, 0, 0));
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0, 0, 0, 0));
	ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 0.0f));

	for (int i = 0; i < 2; ++i) {
		bool is_selected = m_selected_tab == i;
		
		ImGui::PushID(i);
		ImVec2 current_pos = ImGui::GetCursorScreenPos();
		ImVec2 text_size = ImGui::CalcTextSize(top_tabs[i]);
		ImVec2 tab_size(text_size.x, 30.0f * scale);
		
		bool is_hovered = ImGui::IsMouseHoveringRect(current_pos, ImVec2(current_pos.x + tab_size.x, current_pos.y + tab_size.y));
		
		ImGui::PushStyleColor(ImGuiCol_Text, is_selected ? g_menu_gui.accent_color.to_vec4() : (is_hovered ? g_menu_gui.text.to_vec4() : g_menu_gui.text_disabled.to_vec4()));
		
		if (ImGui::Button(top_tabs[i], tab_size)) {
			reset_tab_animation(m_anim, m_selected_tab, i);
		}
		
		ImGui::PopStyleColor();
		
		if (is_selected) {
			ImVec2 p_min = ImGui::GetItemRectMin();
			float line_y = p_min.y + (30.0f * scale + text_size.y) * 0.5f + 3.0f * scale;
			ImGui::GetWindowDrawList()->AddLine(
				ImVec2(p_min.x, line_y),
				ImVec2(p_min.x + text_size.x, line_y),
				g_menu_gui.accent_color.to_im_color(),
				2.0f * scale
			);
		}
		ImGui::PopID();
		ImGui::SameLine(0.0f, 40.0f * scale); // space between tabs
	}
	ImGui::PopStyleVar(2);
	ImGui::PopStyleColor(3);
	ImGui::EndChild();

	// Left sidebar menu items
	ImGui::SetCursorPos(ImVec2(10.0f * scale, 80.0f * scale));
	ImGui::BeginChild("##sidebar", ImVec2(130.0f * scale, size.y - 140.0f * scale), false, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoBackground);
	if (m_selected_tab == static_cast<int>(menu_tab_t::skins)) {
		g_menu_gui.group_title("Skins");
		if (g_menu_gui.tab(ICON_FA_COG, "General", m_selected_skins_tab == static_cast<int>(skins_sidebar_t::general)))
			m_selected_skins_tab = static_cast<int>(skins_sidebar_t::general);
		ImGui::Spacing();
		if (g_menu_gui.tab(ICON_FA_PAINT_BRUSH, "Weapons", m_selected_skins_tab == static_cast<int>(skins_sidebar_t::weapons)))
			m_selected_skins_tab = static_cast<int>(skins_sidebar_t::weapons);
		ImGui::Spacing();
		if (g_menu_gui.tab(ICON_FA_KNIFE_KITCHEN, "Knives", m_selected_skins_tab == static_cast<int>(skins_sidebar_t::knives)))
			m_selected_skins_tab = static_cast<int>(skins_sidebar_t::knives);
		ImGui::Spacing();
		if (g_menu_gui.tab(ICON_FA_PAINT_BRUSH, "Gloves", m_selected_skins_tab == static_cast<int>(skins_sidebar_t::gloves)))
			m_selected_skins_tab = static_cast<int>(skins_sidebar_t::gloves);
		ImGui::Spacing();
		if (g_menu_gui.tab(ICON_FA_USER, "Agent", m_selected_skins_tab == static_cast<int>(skins_sidebar_t::agent)))
			m_selected_skins_tab = static_cast<int>(skins_sidebar_t::agent);
	}
	else if (m_selected_tab == static_cast<int>(menu_tab_t::setting)) {
		g_menu_gui.group_title("Setting");
		if (g_menu_gui.tab(ICON_FA_DATABASE, "Config", m_selected_setting_tab == static_cast<int>(setting_sidebar_t::config)))
			m_selected_setting_tab = static_cast<int>(setting_sidebar_t::config);
		ImGui::Spacing();
		if (g_menu_gui.tab(ICON_FA_HAMMER, "Misc", m_selected_setting_tab == static_cast<int>(setting_sidebar_t::misc)))
			m_selected_setting_tab = static_cast<int>(setting_sidebar_t::misc);
	}
	ImGui::EndChild();

	m_anim += (1.0f - m_anim) * 0.045f;
	g_menu_gui.m_anim = m_anim;

	// Main content area
	ImGui::SetCursorPos(ImVec2(content_x, (80.0f - 5.0f * m_anim) * scale));
	ImGui::BeginChild("##childs", ImVec2(size.x - content_x - content_right_margin, size.y - 90.0f * scale));
	if (m_selected_tab == static_cast<int>(menu_tab_t::skins))
		menu_tabs::draw_skins(m_selected_skins_tab, scale);
	else if (m_selected_tab == static_cast<int>(menu_tab_t::setting))
		menu_tabs::draw_settings(m_selected_setting_tab, scale, m_waiting_for_key);
	ImGui::EndChild();
	ImGui::End();
	ImGui::PopStyleVar();
}

void Menu::on_create_move() {
	if (!m_opened)
		return;
}
