#include "platform/ClientCore.hpp"

#include "ui/tabs/SettingsTab.hpp"
#include "config/ConfigStore.hpp"
#include "runtime/Runtime.hpp"
#include "ui/KeyNames.hpp"
#include "ui/Menu.hpp"
#include "ui/MenuAssets.hpp"
#include "ui/MenuGui.hpp"

#include <cstring>
#include <string>
static void save_selected_or_default() {
	const std::string& selected = ttapp::client::config_store().m_selected_config;
	ttapp::client::config_store().save(selected.empty() ? "default" : selected);
}

static void draw_config_tab(float scale) {
	g_menu_gui.group_box(ICON_FA_DATABASE " Config", ImVec2(ImGui::GetWindowWidth(), ImGui::GetWindowHeight()));

	ttapp::client::config_store().refresh();
	auto& configs = ttapp::client::config_store().get_config_files();

	ImGui::TextUnformatted("Saved Configs");
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10.0f * scale, 8.0f * scale));
	ImGui::BeginChild("config_list", ImVec2(0.0f, 150.0f * scale), true);
	for (const auto& cfg : configs) {
		const bool is_selected = ttapp::client::config_store().m_selected_config == cfg;
		if (ImGui::Selectable(cfg.c_str(), is_selected))
			ttapp::client::config_store().m_selected_config = cfg;
	}
	ImGui::EndChild();
	ImGui::PopStyleVar();

	static char config_name[64] = "";
	ImGui::InputText("Config Name", config_name, sizeof(config_name));

	if (ImGui::Button("Create", ImVec2(80.0f * scale, 0.0f)) && std::strlen(config_name) > 0) {
		ttapp::client::config_store().save(config_name);
		config_name[0] = '\0';
	}
	ImGui::SameLine();

	if (ImGui::Button("Save", ImVec2(80.0f * scale, 0.0f)))
		save_selected_or_default();
	ImGui::SameLine();

	if (ImGui::Button("Load", ImVec2(80.0f * scale, 0.0f)) && !ttapp::client::config_store().m_selected_config.empty())
		ttapp::client::config_store().load(ttapp::client::config_store().m_selected_config);

	// Delete — opens confirmation modal
	if (ImGui::Button("Delete", ImVec2(80.0f * scale, 0.0f)) && !ttapp::client::config_store().m_selected_config.empty())
		ImGui::OpenPopup("##confirm_delete");
	ImGui::SameLine();

	// Reset — opens confirmation modal
	if (ImGui::Button("Reset", ImVec2(80.0f * scale, 0.0f)))
		ImGui::OpenPopup("##confirm_reset");
	ImGui::SameLine();

	if (ImGui::Button("Refresh", ImVec2(80.0f * scale, 0.0f)))
		ttapp::client::config_store().refresh();

	// --- Confirmation: Delete ---
	const ImVec2 window_pos = ImGui::GetWindowPos();
	const ImVec2 window_size = ImGui::GetWindowSize();
	ImVec2 center(window_pos.x + window_size.x * 0.5f, window_pos.y + window_size.y * 0.5f);
	ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
	ImGui::SetNextWindowSize(ImVec2(320.0f * scale, 0.0f));
	ImGui::PushStyleColor(ImGuiCol_PopupBg,          ImVec4(0.10f, 0.10f, 0.12f, 1.00f));
	ImGui::PushStyleColor(ImGuiCol_ModalWindowDimBg, ImVec4(0.00f, 0.00f, 0.00f, 0.70f));
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding,  ImVec2(20.0f * scale, 16.0f * scale));
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing,    ImVec2(8.0f  * scale,  8.0f * scale));
	if (ImGui::BeginPopupModal("##confirm_delete", nullptr,
		ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove))
	{
		// Title row
		ImGui::TextColored(ImVec4(1.0f, 0.75f, 0.2f, 1.0f),
			ICON_FA_EXCLAMATION_TRIANGLE "  Delete config?");
		ImGui::Spacing();

		// Body
		ImGui::TextWrapped("'%s' will be permanently removed.",
			ttapp::client::config_store().m_selected_config.c_str());
		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();

		// Equal-width buttons
		const float btn_w = (ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x) * 0.5f;
		if (ImGui::Button("Yes, delete", ImVec2(btn_w, 28.0f * scale))) {
			ttapp::client::config_store().remove(ttapp::client::config_store().m_selected_config);
			ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine();
		if (ImGui::Button("Cancel", ImVec2(btn_w, 28.0f * scale)))
			ImGui::CloseCurrentPopup();

		ImGui::Spacing();
		ImGui::EndPopup();
	}
	ImGui::PopStyleVar(2);
	ImGui::PopStyleColor(2);

	// --- Confirmation: Reset ---
	ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
	ImGui::SetNextWindowSize(ImVec2(320.0f * scale, 0.0f));
	ImGui::PushStyleColor(ImGuiCol_PopupBg,          ImVec4(0.10f, 0.10f, 0.12f, 1.00f));
	ImGui::PushStyleColor(ImGuiCol_ModalWindowDimBg, ImVec4(0.00f, 0.00f, 0.00f, 0.70f));
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding,  ImVec2(20.0f * scale, 16.0f * scale));
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing,    ImVec2(8.0f  * scale,  8.0f * scale));
	if (ImGui::BeginPopupModal("##confirm_reset", nullptr,
		ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove))
	{
		// Title row
		ImGui::TextColored(ImVec4(1.0f, 0.75f, 0.2f, 1.0f),
			ICON_FA_EXCLAMATION_TRIANGLE "  Reset all settings?");
		ImGui::Spacing();

		// Body
		ImGui::TextWrapped("All settings will be restored to defaults.");
		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();

		// Equal-width buttons
		const float btn_w = (ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x) * 0.5f;
		if (ImGui::Button("Yes, reset", ImVec2(btn_w, 28.0f * scale))) {
			ttapp::client::config_store().reset();
			ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine();
		if (ImGui::Button("Cancel", ImVec2(btn_w, 28.0f * scale)))
			ImGui::CloseCurrentPopup();

		ImGui::Spacing();
		ImGui::EndPopup();
	}
	ImGui::PopStyleVar(2);
	ImGui::PopStyleColor(2);

	g_menu_gui.end_group_box();
}


static void draw_misc_main_tab(float scale, bool& waiting_for_key) {
	g_menu_gui.group_box(ICON_FA_COG " Menu", ImVec2(ImGui::GetWindowWidth(), 95.0f * scale));

	const std::string current_key = get_key_name(ttapp::client::config().misc.m_menu_key);
	ImGui::Text("Current key: %s", current_key.c_str());

	const char* button_label = waiting_for_key ? "Press a key..." : "Change menu key";
	if (ImGui::Button(button_label, ImVec2(200.0f * scale, 25.0f * scale)))
		waiting_for_key = true;

	g_menu_gui.end_group_box();

	g_menu_gui.group_box(ICON_FA_HAMMER " Movement", ImVec2(ImGui::GetWindowWidth(), 65.0f * scale));
	ImGui::Checkbox("Quick stop", &ttapp::client::config().misc.m_quick_stop);
	g_menu_gui.end_group_box();

	g_menu_gui.group_box(ICON_FA_COG " HUD", ImVec2(ImGui::GetWindowWidth(), 65.0f * scale));
	ImGui::Checkbox("Spectator list", &ttapp::client::config().misc.m_spectator_list);
	g_menu_gui.end_group_box();

	g_menu_gui.group_box(ICON_FA_COG " Matchmaking", ImVec2(ImGui::GetWindowWidth(), 100.0f * scale));
	ImGui::Checkbox("Auto accept", &ttapp::client::config().misc.m_auto_accept);
	ImGui::Checkbox("Status text", &ttapp::client::config().misc.m_auto_accept_status_text);
	g_menu_gui.end_group_box();
}
namespace menu_tabs {

void draw_settings(int selected_setting_tab, float scale, bool& waiting_for_key)
{
    switch (selected_setting_tab) {
    case static_cast<int>(setting_sidebar_t::config):
        draw_config_tab(scale);
        break;
    case static_cast<int>(setting_sidebar_t::misc):
        draw_misc_main_tab(scale, waiting_for_key);
        break;
    default:
        break;
    }
}

} // namespace menu_tabs
