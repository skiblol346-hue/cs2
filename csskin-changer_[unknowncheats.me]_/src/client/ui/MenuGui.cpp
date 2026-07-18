#define IMGUI_DEFINE_MATH_OPERATORS
#include "ui/MenuGui.hpp"
#include "imgui_internal.h"

#include <string>
#include <unordered_map>

MenuGui g_menu_gui;

ImColor nl_color_t::to_im_color(float alpha, bool use_style_alpha) const {
	return ImColor(r, g, b, (a * (use_style_alpha ? ImGui::GetStyle().Alpha : 1.0f)) * alpha);
}

ImVec4 nl_color_t::to_vec4(float alpha, bool use_style_alpha) const {
	return ImVec4(r, g, b, (a * (use_style_alpha ? ImGui::GetStyle().Alpha : 1.0f)) * alpha);
}

void MenuGui::group_title(const char* name) {
	ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 10.0f);
	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 0.5f));
	ImGui::TextUnformatted(name);
	ImGui::PopStyleColor();
}

bool MenuGui::tab(const char* icon, const char* label, bool selected) {
	auto* window = ImGui::GetCurrentWindow();
	if (window->SkipItems)
		return false;

	const ImGuiID id = window->GetID(label);
	const ImVec2 label_size = ImGui::CalcTextSize(label, nullptr, true);
	const ImVec2 pos = window->DC.CursorPos;
	auto* draw = window->DrawList;

	const ImRect bb(pos, ImVec2(pos.x + ImGui::GetWindowWidth(), pos.y + 30.0f));
	ImGui::ItemAdd(bb, id);
	ImGui::ItemSize(bb, ImGui::GetStyle().FramePadding.y);

	bool hovered = false;
	bool held = false;
	const bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held);

	static std::unordered_map<ImGuiID, float> values;
	float& value = values[id];
	value = ImLerp(value, selected ? 1.0f : 0.0f, 0.05f);

	draw->AddRectFilled(bb.Min, bb.Max, frame_active.to_im_color(0.3f * value), 5.0f);
	if (value > 0.01f) {
		draw->AddRectFilled(
			ImVec2(bb.Min.x + 2.0f, bb.Min.y + 5.0f),
			ImVec2(bb.Min.x + 5.0f, bb.Max.y - 5.0f),
			accent_color.to_im_color(value),
			1.5f
		);
	}
	draw->AddText(ImVec2(bb.Min.x + 15.0f, bb.GetCenter().y - label_size.y * 0.5f), selected ? accent_color.to_im_color() : text_disabled.to_im_color(), icon);
	draw->AddText(ImVec2(bb.Min.x + 40.0f, bb.GetCenter().y - label_size.y * 0.5f), selected ? text.to_im_color() : text_disabled.to_im_color(), label);

	return pressed;
}

bool MenuGui::subtab(const char* label, bool selected, int count, ImDrawFlags flags) {
	auto* window = ImGui::GetCurrentWindow();
	if (window->SkipItems)
		return false;

	const ImGuiID id = window->GetID(label);
	const ImVec2 label_size = ImGui::CalcTextSize(label, nullptr, true);
	const ImVec2 pos = window->DC.CursorPos;
	auto* draw = window->DrawList;

	const float width = count > 0 ? ImGui::GetWindowWidth() / static_cast<float>(count) : ImGui::GetWindowWidth();
	const ImRect bb(pos, ImVec2(pos.x + width, pos.y + ImGui::GetWindowHeight()));
	ImGui::ItemAdd(bb, id);
	ImGui::ItemSize(bb, ImGui::GetStyle().FramePadding.y);

	bool hovered = false;
	bool held = false;
	const bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held);

	static std::unordered_map<ImGuiID, float> values;
	float& value = values[id];
	value = ImLerp(value, selected ? 1.0f : 0.0f, 0.05f);

	if (hovered || selected) {
		draw->AddRectFilled(bb.Min, bb.Max, button_hovered.to_im_color(0.3f), 4.0f, flags);
	}
	if (value > 0.01f) {
		draw->AddRectFilled(
			ImVec2(bb.Min.x + 8.0f, bb.Max.y - 3.0f),
			ImVec2(bb.Max.x - 8.0f, bb.Max.y),
			accent_color.to_im_color(value),
			1.0f
		);
	}
	draw->AddText(ImVec2(bb.GetCenter().x - label_size.x * 0.5f, bb.GetCenter().y - label_size.y * 0.5f),
		selected ? accent_color.to_im_color() : (hovered ? text.to_im_color() : text_disabled.to_im_color()), label);

	return pressed;
}

void MenuGui::group_box(const char* name, ImVec2 size_arg) {
	auto* window = ImGui::GetCurrentWindow();
	const ImVec2 pos = window->DC.CursorPos;

	ImGui::BeginChild(std::string(name).append(".main").c_str(), size_arg, false, ImGuiWindowFlags_NoScrollbar);

	ImGui::GetWindowDrawList()->AddRectFilled(ImVec2(pos.x, pos.y + 20.0f), ImVec2(pos.x + size_arg.x, pos.y + size_arg.y), group_box_bg.to_im_color(), 6.0f);
	ImGui::GetWindowDrawList()->AddRect(ImVec2(pos.x, pos.y + 20.0f), ImVec2(pos.x + size_arg.x, pos.y + size_arg.y), border.to_im_color(), 6.0f);
	ImGui::GetWindowDrawList()->AddText(ImVec2(pos.x, pos.y), ImGui::GetColorU32(ImGuiCol_Text, 0.5f), name);

	ImGui::SetCursorPos(ImVec2(12.0f, 21.0f));
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 10.0f));
	ImGui::BeginChild(name, ImVec2(size_arg.x - 24.0f, size_arg.y - 21.0f), false, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_AlwaysUseWindowPadding);
	ImGui::BeginGroup();
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8.0f, 10.0f));
	ImGui::PushStyleVar(ImGuiStyleVar_Alpha, m_anim);
}

void MenuGui::end_group_box() {
	ImGui::PopStyleVar(3);
	ImGui::EndGroup();
	ImGui::EndChild();
	ImGui::EndChild();
}
