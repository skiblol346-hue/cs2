#include "platform/ClientCore.hpp"

#include "features/hud/SpectatorList.hpp"
#include "runtime/Runtime.hpp"
#include "ui/Menu.hpp"
#include "ui/MenuGui.hpp"
#include "game/entities/c_cs_player_pawn.hpp"
#include "game/interfaces/i_entity_system.hpp"

#include <algorithm>
#include <cstdarg>
#include <cstdio>
#include <string>
#include <vector>
struct spectator_entry_t {
	std::string name;
};

struct spectator_probe_t {
	const char* name = nullptr;
	uint64_t steam_id = 0;
};

static int g_spectator_trace_frames = 0;
static bool g_spectator_was_enabled = false;
static bool g_spectator_waiting_for_local_logged = false;
static DWORD64 g_spectator_pause_until_ms = 0;
static void* g_spectator_last_entity_system = nullptr;
static void* g_spectator_last_local_pawn = nullptr;
static void* g_spectator_last_local_controller = nullptr;

static void spectator_file_log(const char* format, ...) {
	char log_path[MAX_PATH] = {};
	logger::build_path(log_path, sizeof(log_path), "spectator_list_log.txt");
	FILE* file = nullptr;
	if (fopen_s(&file, log_path, "a") != 0 || !file)
		return;

	fprintf(file, "[%llu] ", static_cast<unsigned long long>(GetTickCount64()));

	va_list args;
	va_start(args, format);
	vfprintf(file, format, args);
	va_end(args);

	fprintf(file, "\n");
	fclose(file);
}

static bool spectator_trace_enabled() {
	return g_spectator_trace_frames > 0;
}

static void finish_spectator_trace_frame(bool trace) {
	if (trace && g_spectator_trace_frames > 0)
		--g_spectator_trace_frames;
}

static void update_spectator_trace_state(bool enabled) {
	if (enabled == g_spectator_was_enabled)
		return;

	g_spectator_was_enabled = enabled;
	if (enabled) {
		g_spectator_trace_frames = 8;
		g_spectator_waiting_for_local_logged = false;
		spectator_file_log("enabled");
	}
	else {
		g_spectator_trace_frames = 0;
		g_spectator_waiting_for_local_logged = false;
		spectator_file_log("disabled");
	}
}

static bool spectator_context_changed(i_entity_system* entity_system, void* local_pawn, void* local_controller) {
	if (entity_system == g_spectator_last_entity_system &&
		local_pawn == g_spectator_last_local_pawn &&
		local_controller == g_spectator_last_local_controller) {
		return false;
	}

	g_spectator_last_entity_system = entity_system;
	g_spectator_last_local_pawn = local_pawn;
	g_spectator_last_local_controller = local_controller;
	g_spectator_pause_until_ms = GetTickCount64() + 2000;
	return true;
}

static bool try_get_local_handle(c_cs_player_pawn* local_pawn, c_base_handle& out, bool trace) {
	__try {
		out = local_pawn->get_handle();
		return true;
	}
	__except (EXCEPTION_EXECUTE_HANDLER) {
		if (trace)
			spectator_file_log("skip: exception reading local_handle");
		return false;
	}
}

static bool try_get_view_target(c_cs_player_pawn* local_pawn, c_base_handle& out, bool trace) {
	__try {
		auto* observer_services = local_pawn->m_observer_services();
		if (!valid_ptr(observer_services))
			return false;

		out = observer_services->m_observer_target();
		return out.is_valid();
	}
	__except (EXCEPTION_EXECUTE_HANDLER) {
		if (trace)
			spectator_file_log("skip: exception reading local observer_target");
		return false;
	}
}

static bool try_get_controller(i_entity_system* entity_system, int slot, c_cs_player_controller*& out, bool trace) {
	__try {
		out = entity_system->get_base_entity<c_cs_player_controller>(slot);
		return true;
	}
	__except (EXCEPTION_EXECUTE_HANDLER) {
		if (trace)
			spectator_file_log("slot %d exception getting controller", slot);
		return false;
	}
}

static bool try_collect_spectator_slot(i_entity_system* entity_system, int slot, c_cs_player_controller* controller, c_cs_player_controller* local_controller, c_cs_player_pawn* local_pawn, c_base_handle target_handle, bool include_local_controller, bool trace, spectator_probe_t& out) {
	__try {
		if (!entity_system || !valid_ptr(controller) || (!include_local_controller && controller == local_controller) || controller == reinterpret_cast<c_cs_player_controller*>(local_pawn))
			return false;

		if (trace)
			spectator_file_log("slot %d before pawn_handle", slot);
		const c_base_handle pawn_handle = controller->m_pawn();
		if (!pawn_handle.is_valid() || pawn_handle.get_entry_index() <= 0)
			return false;

		if (trace)
			spectator_file_log("slot %d before observer_pawn idx=%d", slot, pawn_handle.get_entry_index());
		auto* observer_pawn = entity_system->get_base_entity<c_cs_player_pawn>(pawn_handle.get_entry_index());
		if (!valid_ptr(observer_pawn))
			return false;

		if (trace)
			spectator_file_log("slot %d before alive", slot);
		if (controller->m_pawn_is_alive())
			return false;

		if (trace)
			spectator_file_log("slot %d before pawn observer_services pawn=%p", slot, observer_pawn);
		auto* observer_services = observer_pawn->m_observer_services();
		if (!valid_ptr(observer_services))
			return false;

		if (trace)
			spectator_file_log("slot %d before observer_target services=%p", slot, observer_services);
		if (observer_services->m_observer_target() != target_handle)
			return false;

		if (trace)
			spectator_file_log("slot %d before name", slot);
		out.name = controller->m_player_name();
		if (!out.name || !*out.name)
			return false;

		out.steam_id = controller->m_steam_id();
		return true;
	}
	__except (EXCEPTION_EXECUTE_HANDLER) {
		if (trace)
			spectator_file_log("slot %d exception", slot);
		return false;
	}
}

static std::vector<spectator_entry_t> collect_spectators() {
	std::vector<spectator_entry_t> spectators;

	const bool enabled = ttapp::client::config().misc.m_spectator_list;
	update_spectator_trace_state(enabled);
	if (!enabled)
		return spectators;

	auto* entity_system = ttapp::client::interfaces().m_entity_system;
	void* raw_local_pawn = ttapp::client::frame().m_local_pawn;
	void* raw_local_controller = ttapp::client::frame().m_local_controller;
	const bool trace = spectator_trace_enabled();
	if (trace)
		spectator_file_log("collect begin entity_system=%p local_pawn=%p local_controller=%p",
			entity_system, raw_local_pawn, raw_local_controller);

	if (spectator_context_changed(entity_system, raw_local_pawn, raw_local_controller)) {
		if (trace)
			spectator_file_log("skip: context changed, pausing probe");
		finish_spectator_trace_frame(trace);
		return spectators;
	}

	const DWORD64 now = GetTickCount64();
	if (now < g_spectator_pause_until_ms) {
		if (trace)
			spectator_file_log("skip: paused for map/load transition");
		finish_spectator_trace_frame(trace);
		return spectators;
	}

	if (!entity_system || !raw_local_pawn || !raw_local_controller) {
		if (trace && !g_spectator_waiting_for_local_logged) {
			spectator_file_log("skip: missing entity_system/local");
			g_spectator_waiting_for_local_logged = true;
		}
		finish_spectator_trace_frame(trace);
		return spectators;
	}

	auto* local_pawn = reinterpret_cast<c_cs_player_pawn*>(raw_local_pawn);
	auto* local_controller = reinterpret_cast<c_cs_player_controller*>(raw_local_controller);
	if (!valid_ptr(local_pawn)) {
		if (trace && !g_spectator_waiting_for_local_logged) {
			spectator_file_log("skip: invalid local_pawn=%p", local_pawn);
			g_spectator_waiting_for_local_logged = true;
		}
		g_spectator_pause_until_ms = GetTickCount64() + 2000;
		finish_spectator_trace_frame(trace);
		return spectators;
	}

	if (!valid_ptr(local_controller)) {
		if (trace && !g_spectator_waiting_for_local_logged) {
			spectator_file_log("skip: invalid local_controller=%p", local_controller);
			g_spectator_waiting_for_local_logged = true;
		}
		g_spectator_pause_until_ms = GetTickCount64() + 2000;
		finish_spectator_trace_frame(trace);
		return spectators;
	}

	if (trace && g_spectator_waiting_for_local_logged) {
		spectator_file_log("local pawn ready");
		g_spectator_waiting_for_local_logged = false;
	}

	c_base_handle local_handle;
	if (!try_get_local_handle(local_pawn, local_handle, trace)) {
		g_spectator_pause_until_ms = GetTickCount64() + 2000;
		finish_spectator_trace_frame(trace);
		return spectators;
	}

	if (!local_handle.is_valid()) {
		if (trace)
			spectator_file_log("skip: invalid local_handle");
		finish_spectator_trace_frame(trace);
		return spectators;
	}

	c_base_handle target_handle = local_handle;
	c_base_handle view_target;
	if (try_get_view_target(local_pawn, view_target, trace)) {
		target_handle = view_target;
		if (trace)
			spectator_file_log("using view target idx=%d", target_handle.get_entry_index());
	}
	const bool include_local_controller = target_handle != local_handle;

	std::vector<uint64_t> seen_steam_ids;
	std::vector<std::string> seen_names;

	for (int i = 1; i <= 64; ++i) {
		c_cs_player_controller* controller = nullptr;
		if (!try_get_controller(entity_system, i, controller, trace)) {
			g_spectator_pause_until_ms = GetTickCount64() + 2000;
			break;
		}

		if (trace)
			spectator_file_log("slot %d controller=%p", i, controller);

		spectator_probe_t probe;
		if (!try_collect_spectator_slot(entity_system, i, controller, local_controller, local_pawn, target_handle, include_local_controller, trace, probe))
			continue;

		if (probe.steam_id != 0) {
			if (std::find(seen_steam_ids.begin(), seen_steam_ids.end(), probe.steam_id) != seen_steam_ids.end())
				continue;

			seen_steam_ids.push_back(probe.steam_id);
		}
		else {
			if (std::find(seen_names.begin(), seen_names.end(), probe.name) != seen_names.end())
				continue;

			seen_names.emplace_back(probe.name);
		}

		if (trace)
			spectator_file_log("slot %d spectator name=%s steam_id=%llu", i, probe.name, static_cast<unsigned long long>(probe.steam_id));
		spectators.push_back({ probe.name });
	}

	if (trace)
		spectator_file_log("collect end count=%d", static_cast<int>(spectators.size()));

	finish_spectator_trace_frame(trace);
	return spectators;
}

void SpectatorList::pause(float seconds) {
	const DWORD64 delay_ms = static_cast<DWORD64>(std::max(0.0f, seconds) * 1000.0f);
	g_spectator_pause_until_ms = std::max(g_spectator_pause_until_ms, GetTickCount64() + delay_ms);
	g_spectator_last_entity_system = nullptr;
	g_spectator_last_local_pawn = nullptr;
	g_spectator_last_local_controller = nullptr;
	if (ttapp::client::config().misc.m_spectator_list)
		spectator_file_log("pause requested %.1fs", seconds);
}

void SpectatorList::draw() {
	if (!ttapp::client::config().misc.m_spectator_list)
		return;

	auto spectators = collect_spectators();

	ttapp::client::menu().ensure_style();

	const float scale = ttapp::client::menu().get_dpi_scale();
	auto& misc = ttapp::client::config().misc;
	ImGui::SetNextWindowPos(ImVec2(misc.m_spectator_list_x * scale, misc.m_spectator_list_y * scale), ImGuiCond_FirstUseEver);
	ImGui::PushStyleColor(ImGuiCol_WindowBg, g_menu_gui.group_box_bg.to_vec4());
	ImGui::PushStyleColor(ImGuiCol_Border, g_menu_gui.border.to_vec4());
	ImGui::PushStyleColor(ImGuiCol_Text, g_menu_gui.text.to_vec4());
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10.0f * scale, 8.0f * scale));
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 8.0f * scale);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1.0f);
	ImGui::Begin("##spectator_list", nullptr,
		ImGuiWindowFlags_NoTitleBar |
		ImGuiWindowFlags_AlwaysAutoResize |
		ImGuiWindowFlags_NoSavedSettings);
	const ImVec2 window_pos = ImGui::GetWindowPos();
	misc.m_spectator_list_x = window_pos.x / scale;
	misc.m_spectator_list_y = window_pos.y / scale;

	ImGui::TextUnformatted("Spectators");
	ImGui::SameLine();
	ImGui::TextColored(g_menu_gui.text_disabled.to_vec4(), "%d", static_cast<int>(spectators.size()));
	ImGui::Separator();

	if (spectators.empty()) {
		ImGui::TextColored(g_menu_gui.text_disabled.to_vec4(), "No spectators");
	}
	else {
		for (const auto& spectator : spectators)
			ImGui::TextUnformatted(spectator.name.c_str());
	}

	ImGui::End();
	ImGui::PopStyleVar(3);
	ImGui::PopStyleColor(3);
}
