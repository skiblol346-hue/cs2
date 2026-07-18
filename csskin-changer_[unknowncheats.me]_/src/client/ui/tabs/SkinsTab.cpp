#include "platform/ClientCore.hpp"

#include "ui/tabs/SkinsTab.hpp"
#include "runtime/Runtime.hpp"
#include "ui/Menu.hpp"
#include "ui/MenuAssets.hpp"
#include "ui/MenuGui.hpp"
#include "game/items/ItemSchema.hpp"
#include "features/skins/AgentChanger.hpp"
#include "features/skins/GloveChanger.hpp"
#include "features/skins/SkinChanger.hpp"
#include "game/interfaces/i_panorama.hpp"

#include <algorithm>
#include <cctype>
#include <string>
#include <vector>
static ImU32 get_rarity_color(int rarity) {
	switch (rarity) {
	case 1: return IM_COL32(176, 195, 217, 255);
	case 2: return IM_COL32(94, 152, 217, 255);
	case 3: return IM_COL32(75, 105, 255, 255);
	case 4: return IM_COL32(136, 71, 255, 255);
	case 5: return IM_COL32(211, 44, 230, 255);
	case 6: return IM_COL32(235, 75, 75, 255);
	case 7: return IM_COL32(228, 174, 57, 255);
	default: return IM_COL32(255, 255, 255, 255);
	}
}

static bool draw_paint_kit_combo(const char* label, uint16_t definition_index, int& selected_kit) {
	auto& schema = ttapp::client::item_schema();
	schema.ensure_paint_kits_for_item(definition_index);
	auto kits_it = schema.item_paint_kits.find(definition_index);
	if (kits_it == schema.item_paint_kits.end() || kits_it->second.empty()) {
		ImGui::TextDisabled("Loading skins...");
		return false;
	}

	bool changed = false;
	auto& kits = kits_it->second;
	const char* preview_value = selected_kit >= 0 && selected_kit < static_cast<int>(kits.size())
		? kits[selected_kit].name.c_str()
		: "";

	if (ImGui::BeginCombo(label, preview_value)) {
		for (int i = 0; i < static_cast<int>(kits.size()); ++i) {
			const bool is_selected = selected_kit == i;
			ImGui::PushID(i);
			if (ImGui::Selectable(kits[i].name.c_str(), is_selected)) {
				selected_kit = i;
				changed = true;
			}

			const ImVec2 p_min = ImGui::GetItemRectMin();
			const ImVec2 p_max = ImGui::GetItemRectMax();
			ImGui::GetWindowDrawList()->AddRectFilled(
				ImVec2(p_min.x + 1.0f, p_min.y + 1.0f),
				ImVec2(p_min.x + 4.0f, p_max.y - 1.0f),
				get_rarity_color(kits[i].rarity)
			);

			if (is_selected)
				ImGui::SetItemDefaultFocus();
			ImGui::PopID();
		}
		ImGui::EndCombo();
	}

	return changed;
}

static bool draw_agent_combo(const char* label, const std::vector<item_info_t>& agents, int& selected_agent) {
	if (agents.empty()) {
		ImGui::TextDisabled("Loading agents...");
		return false;
	}

	bool changed = false;
	const char* preview_value = selected_agent >= 0 && selected_agent < static_cast<int>(agents.size())
		? agents[selected_agent].name.c_str()
		: "";

	if (ImGui::BeginCombo(label, preview_value)) {
		for (int i = 0; i < static_cast<int>(agents.size()); ++i) {
			const bool is_selected = selected_agent == i;
			ImGui::PushID(i);
			if (ImGui::Selectable(agents[i].name.c_str(), is_selected)) {
				selected_agent = i;
				changed = true;
			}

			const ImVec2 p_min = ImGui::GetItemRectMin();
			const ImVec2 p_max = ImGui::GetItemRectMax();
			ImGui::GetWindowDrawList()->AddRectFilled(
				ImVec2(p_min.x + 1.0f, p_min.y + 1.0f),
				ImVec2(p_min.x + 4.0f, p_max.y - 1.0f),
				get_rarity_color(agents[i].rarity)
			);

			if (is_selected)
				ImGui::SetItemDefaultFocus();
			ImGui::PopID();
		}
		ImGui::EndCombo();
	}

	return changed;
}

static bool draw_wear_seed(float& wear, int& seed, const char* suffix) {
	const float item_width = ImGui::CalcItemWidth();
	const float half_width = (item_width - ImGui::GetStyle().ItemSpacing.x) * 0.5f;
	bool changed = false;

	ImGui::SetNextItemWidth(half_width);
	changed |= ImGui::SliderFloat((std::string("Wear##") + suffix).c_str(), &wear, 0.0f, 1.0f, "%.4f");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(half_width);
	changed |= ImGui::InputInt((std::string("Seed##") + suffix).c_str(), &seed, 0, 0);

	return changed;
}

static bool draw_color4_controls(const char* id, bool& enabled, float colors[4][4], float scale) {
	bool changed = false;

	ImGui::AlignTextToFramePadding();
	ImGui::Text("Custom Color");
	ImGui::SameLine(100.0f * scale);
	changed |= ImGui::Checkbox((std::string("##") + id + "_enabled").c_str(), &enabled);

	if (enabled) {
		for (int c = 0; c < 4; ++c) {
			ImGui::AlignTextToFramePadding();
			ImGui::Text("Color %d", c + 1);
			ImGui::SameLine(100.0f * scale);
			ImGui::SetNextItemWidth(180.0f * scale);
			changed |= ImGui::ColorEdit4((std::string("##") + id + "_" + std::to_string(c)).c_str(),
				colors[c],
				ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar);
		}
	}

	return changed;
}

static std::string lower_copy(std::string value) {
	std::transform(value.begin(), value.end(), value.begin(), [](unsigned char ch) {
		return static_cast<char>(std::tolower(ch));
		});
	return value;
}

static ImVec2 cosmetic_popup_max_size(float scale) {
	const ImVec2 display = ImGui::GetIO().DisplaySize;
	float max_x = 700.0f * scale;
	float max_y = 620.0f * scale;

	if (display.x > 0.0f)
		max_x = std::min(max_x, display.x - 40.0f * scale);
	if (display.y > 0.0f)
		max_y = std::min(max_y, display.y - 80.0f * scale);

	return ImVec2(std::max(480.0f * scale, max_x), std::max(360.0f * scale, max_y));
}

static CImageProxySource* load_cosmetic_image(const cosmetic_item_info_t& item) {
	if (item.image_inventory.empty())
		return nullptr;
	if (item.m_image)
		return item.m_image;

	auto& interfaces = ttapp::client::interfaces();
	if (!interfaces.m_file_system)
		return nullptr;

	const std::string game_path = "panorama/images/" + item.image_inventory + "_png.vtex_c";
	if (!interfaces.m_file_system->exists(game_path.c_str(), "GAME"))
		return nullptr;

	auto* panorama = interfaces.m_panorama;
	if (!panorama)
		return nullptr;

	auto* ui_engine = panorama->AccessUIEngine();
	if (!ui_engine)
		return nullptr;

	auto* resource_mgr = ui_engine->GetResourceManager();
	if (!resource_mgr)
		return nullptr;

	const std::string load_path = "s2r://panorama/images/" + item.image_inventory + "_png.vtex";
	item.m_image = resource_mgr->LoadImageInternal(load_path.c_str(), EImageFormat::RGBA8888);
	return item.m_image;
}

// tile_w x tile_h — image fills the top square portion, name label strip at the bottom
static bool draw_cosmetic_tile(const cosmetic_item_info_t& item, int selected_id, float tile_w, float tile_h, float scale) {
	bool clicked = false;
	ImGui::PushID(item.id);

	const ImVec2 cursor = ImGui::GetCursorScreenPos();
	if (ImGui::InvisibleButton("##cosmetic_tile", ImVec2(tile_w, tile_h)))
		clicked = true;

	const bool hovered = ImGui::IsItemHovered();
	const bool selected = selected_id == item.id;
	const ImVec2 min = ImGui::GetItemRectMin();
	const ImVec2 max = ImGui::GetItemRectMax();
	ImDrawList* draw = ImGui::GetWindowDrawList();

	// Background
	const ImU32 bg_color = selected
		? IM_COL32(
			static_cast<int>(g_menu_gui.accent_color.r * 255 * 0.16f),
			static_cast<int>(g_menu_gui.accent_color.g * 255 * 0.16f),
			static_cast<int>(g_menu_gui.accent_color.b * 255 * 0.16f),
			255)
		: (hovered ? IM_COL32(36, 38, 48, 255) : IM_COL32(18, 19, 25, 255));
	draw->AddRectFilled(min, max, bg_color, 6.0f * scale);

	// Border: neutral at rest (no rarity noise), rarity on hover, accent on select
	ImU32 rarity = get_rarity_color(item.rarity);
	const ImU32 border_color = selected
		? ImGui::ColorConvertFloat4ToU32(g_menu_gui.accent_color.to_vec4())
		: (hovered
			? ((rarity & 0x00FFFFFFu) | (static_cast<ImU32>(200) << 24))
			: IM_COL32(45, 47, 60, 255));
	const float border_w = selected ? 2.0f * scale : 1.0f * scale;
	draw->AddRect(min, max, border_color, 6.0f * scale, 0, border_w);

	// Selection glow ring
	if (selected) {
		const ImU32 glow = (ImGui::ColorConvertFloat4ToU32(g_menu_gui.accent_color.to_vec4()) & 0x00FFFFFFu) | (static_cast<ImU32>(35) << 24);
		draw->AddRect(
			ImVec2(min.x - 2.0f * scale, min.y - 2.0f * scale),
			ImVec2(max.x + 2.0f * scale, max.y + 2.0f * scale),
			glow, 8.0f * scale, 0, 4.0f * scale);
	}

	// Image area — leave room for the label strip at the bottom
	const float label_h = 16.0f * scale;
	const float img_area_h = tile_h - label_h;

	CImageProxySource* image = load_cosmetic_image(item);
	ID3D11ShaderResourceView* texture = image ? image->GetNativeTexture() : nullptr;
	if (texture) {
		const ImVec2 image_size = image->GetImageSize();
		const float pad = 5.0f * scale;
		const float avail_w = tile_w - pad * 2.0f;
		const float avail_h = img_area_h - pad * 2.0f;
		float fit = 1.0f;
		if (image_size.x > 0.0f && image_size.y > 0.0f)
			fit = std::min(avail_w / image_size.x, avail_h / image_size.y);
		const ImVec2 draw_size(image_size.x * fit, image_size.y * fit);
		const ImVec2 image_min(
			cursor.x + (tile_w     - draw_size.x) * 0.5f,
			cursor.y + (img_area_h - draw_size.y) * 0.5f);
		draw->AddImageRounded((ImTextureID)texture, image_min,
			ImVec2(image_min.x + draw_size.x, image_min.y + draw_size.y),
			ImVec2(0, 0), ImVec2(1, 1), IM_COL32_WHITE, 3.0f * scale);
	}
	else {
		// No texture yet — show ID centered in image area
		const std::string id_text = std::to_string(item.id);
		const ImVec2 text_size = ImGui::CalcTextSize(id_text.c_str());
		draw->AddText(
			ImVec2(cursor.x + (tile_w - text_size.x) * 0.5f,
				cursor.y + (img_area_h - text_size.y) * 0.5f),
			g_menu_gui.text_disabled.to_im_color(), id_text.c_str());
	}

	// Thin separator between image area and name strip
	draw->AddLine(
		ImVec2(min.x + 4.0f * scale, cursor.y + img_area_h),
		ImVec2(max.x - 4.0f * scale, cursor.y + img_area_h),
		IM_COL32(255, 255, 255, 14), 1.0f);

	// Name label strip — hard-clipped inside tile, truncated with ellipsis
	{
		const float strip_y = cursor.y + img_area_h + 2.0f * scale;
		const float max_label_w = tile_w - 6.0f * scale;
		const char* name = item.name.c_str();

		// IMPORTANT: use plain single-line CalcTextSize (no wrap_width argument)
		// so the true untruncated width is compared against max_label_w.
		// CalcTextSize with a wrap_width returns the wrapped (multi-line) size
		// which is always <= max_label_w — the guard would never fire.
		const float full_w = ImGui::CalcTextSize(name).x;
		const char* name_end = nullptr;
		if (full_w > max_label_w) {
			const float dot_w = ImGui::CalcTextSize("...").x;
			const float budget = max_label_w - dot_w;
			name_end = name;
			float used = 0.0f;
			while (*name_end) {
				const char* next = name_end + 1;
				const float cw = ImGui::CalcTextSize(name_end, next).x;
				if (used + cw > budget)
					break;
				used += cw;
				name_end = next;
			}
		}

		const float drawn_w = ImGui::CalcTextSize(name, name_end).x;
		const float total_w = drawn_w + (name_end && *name_end ? ImGui::CalcTextSize("...").x : 0.0f);
		const float label_x = cursor.x + (tile_w - total_w) * 0.5f;
		const ImU32 label_col = selected
			? ImGui::ColorConvertFloat4ToU32(g_menu_gui.accent_color.to_vec4())
			: static_cast<ImU32>(g_menu_gui.text_disabled.to_im_color(hovered ? 1.0f : 0.72f));

		// Hard clip so no text can ever leak outside the tile rect
		draw->PushClipRect(
			ImVec2(min.x + 2.0f, cursor.y + img_area_h),
			ImVec2(max.x - 2.0f, max.y),
			true);
		draw->AddText(ImVec2(label_x, strip_y), label_col, name, name_end);
		if (name_end && *name_end)
			draw->AddText(ImVec2(label_x + drawn_w, strip_y), label_col, "...");
		draw->PopClipRect();
	}

	if (hovered) {
		ImGui::BeginTooltip();
		ImGui::TextUnformatted(item.name.c_str());
		ImGui::Text("ID: %d", item.id);
		if (!item.internal_name.empty())
			ImGui::TextDisabled("%s", item.internal_name.c_str());
		ImGui::EndTooltip();
	}

	ImGui::PopID();
	return clicked;
}

static bool draw_cosmetic_grid(const std::vector<cosmetic_item_info_t>& items, int& selected_id, const char* search_text, float scale) {
	struct cache_t {
		const cosmetic_item_info_t* data = nullptr;
		std::size_t size = 0;
		std::string search;
		std::vector<std::string> search_blobs;
		std::vector<const cosmetic_item_info_t*> filtered;
	};

	static cache_t cache;
	const cosmetic_item_info_t* data = items.empty() ? nullptr : items.data();
	const std::string search = lower_copy(search_text ? search_text : "");

	if (cache.data != data || cache.size != items.size()) {
		cache.data = data;
		cache.size = items.size();
		cache.search = "\x1f";
		cache.search_blobs.clear();
		cache.search_blobs.reserve(items.size());
		for (const auto& item : items) {
			std::string blob = item.name + "\n" + item.internal_name + "\n" + std::to_string(item.id);
			cache.search_blobs.push_back(lower_copy(std::move(blob)));
		}
		cache.filtered.clear();
	}

	if (cache.search != search) {
		cache.search = search;
		cache.filtered.clear();
		cache.filtered.reserve(items.size());
		for (std::size_t i = 0; i < items.size(); ++i) {
			if (search.empty() || cache.search_blobs[i].find(search) != std::string::npos)
				cache.filtered.push_back(&items[i]);
		}
	}

	if (cache.filtered.empty()) {
		ImGui::TextDisabled(items.empty() ? "Catalog unavailable" : "No matches");
		return false;
	}

	bool changed = false;
	const float spacing = 4.0f * scale;  // fixed inter-tile gap

	// Tiles expand to fill the available row width exactly — no dead space on the right.
	const float avail_w    = ImGui::GetContentRegionAvail().x;
	const float min_tile_w = 82.0f * scale;
	const int   columns    = std::max(1, static_cast<int>((avail_w + spacing) / (min_tile_w + spacing)));
	const float tile_w     = (avail_w - spacing * (columns - 1)) / static_cast<float>(columns);
	const float tile_h     = tile_w + 14.0f * scale;  // square image area + label strip

	const int   rows       = (static_cast<int>(cache.filtered.size()) + columns - 1) / columns;
	const float start_x    = ImGui::GetCursorPosX();
	const float start_y    = ImGui::GetCursorPosY();
	const float row_height = tile_h + spacing;

	ImGuiListClipper clipper;
	clipper.Begin(rows, row_height);
	while (clipper.Step()) {
		for (int row = clipper.DisplayStart; row < clipper.DisplayEnd; ++row) {
			const float row_y = start_y + row * row_height;
			for (int col = 0; col < columns; ++col) {
				const int index = row * columns + col;
				if (index >= static_cast<int>(cache.filtered.size()))
					break;

				ImGui::SetCursorPosX(start_x + col * (tile_w + spacing));
				ImGui::SetCursorPosY(row_y);
				if (draw_cosmetic_tile(*cache.filtered[index], selected_id, tile_w, tile_h, scale)) {
					selected_id = cache.filtered[index]->id;
					changed = true;
				}
			}
		}
	}

	ImGui::SetCursorPosY(start_y + rows * row_height);
	return changed;
}

static void draw_selected_cosmetic_text(const char* label, int id, const cosmetic_item_info_t* item) {
	std::string value = "None";
	if (id > 0)
		value = item ? (item->name + " (" + std::to_string(id) + ")") : ("ID " + std::to_string(id));

	ImGui::Text("%s: %s", label, value.c_str());
}

// Helper: draw a small thumbnail of a cosmetic inside a bounding rect (no button, draw-list only)
static void draw_cosmetic_mini_thumb(ImDrawList* dl, const cosmetic_item_info_t* item, ImVec2 box_min, ImVec2 box_max) {
	if (!item)
		return;
	CImageProxySource* image = load_cosmetic_image(const_cast<cosmetic_item_info_t&>(*item));
	ID3D11ShaderResourceView* texture = image ? image->GetNativeTexture() : nullptr;
	if (!texture)
		return;
	const ImVec2 image_size = image->GetImageSize();
	const float avail_w = box_max.x - box_min.x;
	const float avail_h = box_max.y - box_min.y;
	if (image_size.x <= 0.0f || image_size.y <= 0.0f)
		return;
	const float fit = std::min(avail_w / image_size.x, avail_h / image_size.y);
	const ImVec2 draw_size(image_size.x * fit, image_size.y * fit);
	const ImVec2 img_min(
		box_min.x + (avail_w - draw_size.x) * 0.5f,
		box_min.y + (avail_h - draw_size.y) * 0.5f);
	dl->AddImage((ImTextureID)texture, img_min, ImVec2(img_min.x + draw_size.x, img_min.y + draw_size.y));
}

static bool draw_sticker_editor(Config::skin_changer_t::weapon_skin_t& weapon_skin, float scale) {
	bool changed = false;

	ImGui::AlignTextToFramePadding();
	ImGui::Text("Stickers");
	ImGui::SameLine(100.0f * scale);
	if (ImGui::Button("Manage##stickers", ImVec2(180.0f * scale, 0.0f)))
		ImGui::OpenPopup("##stickers_popup");

	ImGui::SetNextWindowSizeConstraints(ImVec2(480.0f * scale, 380.0f * scale), cosmetic_popup_max_size(scale));
	if (ImGui::BeginPopup("##stickers_popup")) {
		auto& schema = ttapp::client::item_schema();
		const auto& stickers = schema.get_sticker_kits();
		static int selected_slot = 0;
		static char sticker_search[64] = "";
		const int slot_count = Config::skin_changer_t::sticker_slot_count;

		if (selected_slot < 0 || selected_slot >= slot_count)
			selected_slot = 0;

		// ---- Full-width segmented slot control ----
		// Each segment = equal share of popup width. Slot number badge top-right,
		// thumbnail centered, active = accent border + bottom bar.
		{
			const float popup_w    = ImGui::GetContentRegionAvail().x;
			const float seg_gap    = 4.0f * scale;
			const float seg_w      = (popup_w - seg_gap * (slot_count - 1)) / slot_count;
			const float seg_h      = 54.0f * scale;
			const float thumb_pad  = 5.0f * scale;
			const float badge_h    = ImGui::GetTextLineHeight();

			for (int slot = 0; slot < slot_count; ++slot) {
				if (slot > 0)
					ImGui::SameLine(0.0f, seg_gap);

				const bool is_active = (slot == selected_slot);
				const auto& slot_sticker = weapon_skin.stickers[slot];

				// Use direct linear search instead of schema.find_sticker_kit().
				// find_sticker_kit may key on sticker-kit index or name rather than
				// the numeric id stored in config, which causes nullptr on load.
				const cosmetic_item_info_t* slot_item = nullptr;
				if (slot_sticker.id > 0) {
					for (const auto& s : stickers) {
						if (s.id == slot_sticker.id) {
							slot_item = &s;
							break;
						}
					}
				}
				const bool has_sticker = (slot_item != nullptr);

				ImGui::PushID(slot);
				const ImVec2 sp = ImGui::GetCursorScreenPos();
				ImGui::InvisibleButton("##seg", ImVec2(seg_w, seg_h));
				const bool seg_hov = ImGui::IsItemHovered();
				if (ImGui::IsItemClicked())
					selected_slot = slot;

				ImDrawList* dl = ImGui::GetWindowDrawList();
				const ImVec2 sm(sp.x + seg_w, sp.y + seg_h);

				// Background
				const ImU32 seg_bg = is_active
					? IM_COL32(
						static_cast<int>(g_menu_gui.accent_color.r * 255 * 0.14f),
						static_cast<int>(g_menu_gui.accent_color.g * 255 * 0.14f),
						static_cast<int>(g_menu_gui.accent_color.b * 255 * 0.14f),
						255)
					: (seg_hov ? IM_COL32(30, 32, 42, 255) : IM_COL32(15, 16, 21, 255));
				dl->AddRectFilled(sp, sm, seg_bg, 6.0f * scale);

				// Border: accent at reduced alpha when active (avoids neon harshness),
				// dim neutral when empty, rarity hint when filled.
				ImU32 seg_border;
				if (is_active)
					seg_border = (ImGui::ColorConvertFloat4ToU32(g_menu_gui.accent_color.to_vec4()) & 0x00FFFFFFu) | (static_cast<ImU32>(130) << 24);
				else if (has_sticker)
					seg_border = (get_rarity_color(slot_item->rarity) & 0x00FFFFFFu) | (static_cast<ImU32>(90) << 24);
				else
					seg_border = IM_COL32(38, 40, 52, 255);
				dl->AddRect(sp, sm, seg_border, 6.0f * scale, 0, 1.0f * scale);

				// Thumbnail or empty-slot cross
				// c_max goes to sm.y - thumb_pad (no bar_h cutout — bottom bar removed)
				const ImVec2 c_min(sp.x + thumb_pad, sp.y + thumb_pad);
				const ImVec2 c_max(sm.x - thumb_pad, sm.y - thumb_pad);
				dl->PushClipRect(sp, sm, true);
				if (has_sticker) {
					draw_cosmetic_mini_thumb(dl, slot_item, c_min, c_max);
				}
				else {
					const float cx  = sp.x + seg_w * 0.5f;
					const float cy  = sp.y + seg_h * 0.5f;
					const float arm = 5.5f * scale;
					const ImU32 pc  = IM_COL32(58, 62, 78, 255);
					dl->AddLine(ImVec2(cx - arm, cy), ImVec2(cx + arm, cy), pc, 1.5f * scale);
					dl->AddLine(ImVec2(cx, cy - arm), ImVec2(cx, cy + arm), pc, 1.5f * scale);
				}
				dl->PopClipRect();

				// Slot number — top-right corner
				{
					const std::string num = std::to_string(slot);
					const ImVec2 num_sz = ImGui::CalcTextSize(num.c_str());
					const ImU32 nc = is_active
						? ImGui::ColorConvertFloat4ToU32(g_menu_gui.accent_color.to_vec4())
						: static_cast<ImU32>(g_menu_gui.text_disabled.to_im_color(0.45f));
					dl->AddText(ImVec2(sm.x - num_sz.x - 4.0f * scale, sp.y + 3.0f * scale), nc, num.c_str());
				}

				// Hover tooltip for filled slots
				if (seg_hov && has_sticker) {
					ImGui::BeginTooltip();
					ImGui::TextUnformatted(slot_item->name.c_str());
					ImGui::EndTooltip();
				}

				ImGui::PopID();
			}
		}

		ImGui::Spacing();

		auto& sticker = weapon_skin.stickers[selected_slot];
		ImGui::SetNextItemWidth(-1.0f);
		ImGui::InputTextWithHint("##sticker_search", "Search stickers or ID...", sticker_search, sizeof(sticker_search));

		ImGui::BeginChild("##sticker_grid", ImVec2(0.0f, 320.0f * scale), true);
		changed |= draw_cosmetic_grid(stickers, sticker.id, sticker_search, scale);
		ImGui::EndChild();

		if (ImGui::CollapsingHeader("Placement##sticker_placement")) {
			ImGui::SetNextItemWidth(180.0f * scale);
			changed |= ImGui::SliderFloat("Wear##sticker_wear", &sticker.wear, 0.0f, 1.0f, "%.4f");
			ImGui::SetNextItemWidth(180.0f * scale);
			changed |= ImGui::InputFloat("Scale##sticker_scale", &sticker.scale, 0.0f, 0.0f, "%.3f");
			ImGui::SetNextItemWidth(180.0f * scale);
			changed |= ImGui::InputFloat("Rotation##sticker_rotation", &sticker.rotation, 0.0f, 0.0f, "%.3f");
			ImGui::SetNextItemWidth(180.0f * scale);
			changed |= ImGui::InputFloat("Offset X##sticker_offset_x", &sticker.offset_x, 0.0f, 0.0f, "%.3f");
			ImGui::SetNextItemWidth(180.0f * scale);
			changed |= ImGui::InputFloat("Offset Y##sticker_offset_y", &sticker.offset_y, 0.0f, 0.0f, "%.3f");
		}

		if (ImGui::CollapsingHeader("Advanced ID##sticker_advanced")) {
			ImGui::SetNextItemWidth(180.0f * scale);
			changed |= ImGui::InputInt("Manual ID##sticker_id", &sticker.id, 0, 0);
		}

		if (ImGui::Button("Clear Slot##sticker_clear", ImVec2(180.0f * scale, 0.0f))) {
			sticker = Config::skin_changer_t::sticker_slot_t{};
			changed = true;
		}

		ImGui::EndPopup();
	}

	return changed;
}

static bool draw_keychain_editor(Config::skin_changer_t::weapon_skin_t& weapon_skin, float scale) {
	bool changed = false;

	ImGui::AlignTextToFramePadding();
	ImGui::Text("Keychains");
	ImGui::SameLine(100.0f * scale);
	if (ImGui::Button("Manage##keychains", ImVec2(180.0f * scale, 0.0f)))
		ImGui::OpenPopup("##keychains_popup");

	ImGui::SetNextWindowSizeConstraints(ImVec2(480.0f * scale, 360.0f * scale), cosmetic_popup_max_size(scale));
	if (ImGui::BeginPopup("##keychains_popup")) {
		auto& schema = ttapp::client::item_schema();
		const auto& keychains = schema.get_keychain_definitions();
		auto& keychain = weapon_skin.keychain;
		static char keychain_search[64] = "";

		draw_selected_cosmetic_text("Selected", keychain.id, schema.find_keychain_definition(keychain.id));
		ImGui::SetNextItemWidth(-1.0f);
		ImGui::InputTextWithHint("##keychain_search", "Search keychains or ID...", keychain_search, sizeof(keychain_search));

		ImGui::BeginChild("##keychain_grid", ImVec2(0.0f, 320.0f * scale), true);
		changed |= draw_cosmetic_grid(keychains, keychain.id, keychain_search, scale);
		ImGui::EndChild();

		if (ImGui::CollapsingHeader("Placement##keychain_placement")) {
			ImGui::SetNextItemWidth(180.0f * scale);
			changed |= ImGui::InputInt("Seed##keychain_seed", &keychain.seed, 0, 0);
			ImGui::SetNextItemWidth(180.0f * scale);
			changed |= ImGui::InputFloat("Offset X##keychain_offset_x", &keychain.offset_x, 0.0f, 0.0f, "%.3f");
			ImGui::SetNextItemWidth(180.0f * scale);
			changed |= ImGui::InputFloat("Offset Y##keychain_offset_y", &keychain.offset_y, 0.0f, 0.0f, "%.3f");
			ImGui::SetNextItemWidth(180.0f * scale);
			changed |= ImGui::InputFloat("Offset Z##keychain_offset_z", &keychain.offset_z, 0.0f, 0.0f, "%.3f");
		}

		if (ImGui::CollapsingHeader("Advanced ID##keychain_advanced")) {
			ImGui::SetNextItemWidth(180.0f * scale);
			changed |= ImGui::InputInt("Manual ID##keychain_id", &keychain.id, 0, 0);
		}

		if (ImGui::Button("Clear Keychain##keychain_clear", ImVec2(180.0f * scale, 0.0f))) {
			keychain = Config::skin_changer_t::keychain_t{};
			changed = true;
		}

		ImGui::EndPopup();
	}

	return changed;
}

static void draw_skins_general_tab(float scale) {
	g_menu_gui.group_box(ICON_FA_COG " General Settings", ImVec2(750.0f * scale, 450.0f * scale));

	if (ImGui::Checkbox("Skins Enabled", &ttapp::client::config().skin_changer.m_enabled))
		ttapp::client::skin_changer().should_update = true;
	ImGui::Spacing();

	if (ImGui::Checkbox("Knives Enabled", &ttapp::client::config().knife_changer.m_enabled))
		ttapp::client::skin_changer().should_update = true;
	ImGui::Spacing();

	if (ImGui::Checkbox("Gloves Enabled", &ttapp::client::config().glove_changer.m_enabled))
		ttapp::client::glove_changer().should_update = true;
	ImGui::Spacing();

	if (ImGui::Checkbox("Agents Enabled", &ttapp::client::config().agent_changer.m_enabled))
		ttapp::client::agent_changer().should_update = true;

	g_menu_gui.end_group_box();
}

static void draw_skins_weapons_tab(float scale) {
	auto& schema = ttapp::client::item_schema();
	if (!schema.is_initialized()) {
		ImGui::TextDisabled("Loading weapon schema...");
		return;
	}

	// Column 1: Models (categorized)
	ImGui::BeginChild("##weapon_models_col", ImVec2(200.0f * scale, 450.0f * scale), false, ImGuiWindowFlags_NoScrollbar);
	g_menu_gui.group_title("Models");
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(5.0f * scale, 5.0f * scale));
	ImGui::BeginChild("##weapon_models_list", ImVec2(190.0f * scale, 410.0f * scale), true);

	// Category display order and accent colors
	struct WeaponCategoryMeta {
		const char* name;
		ImU32 color;
	};
	static const WeaponCategoryMeta k_weapon_categories[] = {
		{ "Pistol",      IM_COL32(100, 180, 255, 255) },
		{ "Rifle",       IM_COL32( 80, 210, 110, 255) },
		{ "SMG",         IM_COL32(255, 175,  50, 255) },
		{ "Sniper",      IM_COL32(190,  80, 255, 255) },
		{ "Shotgun",     IM_COL32(255,  80,  80, 255) },
		{ "Machine Gun", IM_COL32(255, 205,  40, 255) },
		{ "Other",       IM_COL32(150, 150, 150, 255) },
	};

	ImDrawList* wdl = ImGui::GetWindowDrawList();

	for (int ci = 0; ci < 7; ++ci) {
		const auto& meta = k_weapon_categories[ci];

		// Skip empty categories
		bool has_any = false;
		for (auto& w : schema.weapons)
			if (w.category == meta.name) { has_any = true; break; }
		if (!has_any) continue;

		// --- Category header ---
		ImVec2 hdr_cursor = ImGui::GetCursorScreenPos();
		float avail_w     = ImGui::GetContentRegionAvail().x;
		const float hdr_h = 17.0f * scale;

		// Background + left bar
		wdl->AddRectFilled(hdr_cursor, ImVec2(hdr_cursor.x + avail_w, hdr_cursor.y + hdr_h),
		                   IM_COL32(28, 28, 34, 220), 2.0f * scale);
		wdl->AddRectFilled(hdr_cursor, ImVec2(hdr_cursor.x + 3.0f * scale, hdr_cursor.y + hdr_h),
		                   meta.color);

		ImGui::PushID(ci + 5000);
		ImGui::InvisibleButton("##cat_hdr", ImVec2(avail_w, hdr_h));
		if (ImGui::IsItemClicked()) ttapp::client::config().skin_changer.weapon_cat_open[ci] = !ttapp::client::config().skin_changer.weapon_cat_open[ci];
		ImGui::PopID();

		// Arrow indicator and label
		float text_y = hdr_cursor.y + (hdr_h - ImGui::GetTextLineHeight()) * 0.5f;
		const char* arrow = ttapp::client::config().skin_changer.weapon_cat_open[ci] ? "v " : "> ";
		wdl->AddText(ImVec2(hdr_cursor.x + 6.0f * scale, text_y), meta.color, arrow);
		wdl->AddText(ImVec2(hdr_cursor.x + 18.0f * scale, text_y), IM_COL32(215, 215, 215, 255), meta.name);

		if (!ttapp::client::config().skin_changer.weapon_cat_open[ci]) continue;

		// --- Weapons under this category ---
		ImGui::Indent(8.0f * scale);
		for (int i = 0; i < static_cast<int>(schema.weapons.size()); ++i) {
			if (schema.weapons[i].category != meta.name) continue;

			const bool is_selected = ttapp::client::config().skin_changer.m_selected_weapon == i;

			ImGui::PushID(i);
			if (ImGui::Selectable(schema.weapons[i].name.c_str(), is_selected, 0)) {
				ttapp::client::config().skin_changer.m_selected_weapon = i;
				ttapp::client::skin_changer().should_update = true;
			}
			// Category-colored left stripe per item
			const ImVec2 it_min = ImGui::GetItemRectMin();
			const ImVec2 it_max = ImGui::GetItemRectMax();
			wdl->AddRectFilled(
				ImVec2(it_min.x - 7.0f * scale, it_min.y + 2.0f),
				ImVec2(it_min.x - 4.0f * scale, it_max.y - 2.0f),
				meta.color
			);
			ImGui::PopID();
		}
		ImGui::Unindent(8.0f * scale);
	}

	ImGui::EndChild();
	ImGui::PopStyleVar();
	ImGui::EndChild();

	ImGui::SameLine(0.0f, 10.0f * scale);

	// Get currently selected weapon def index
	uint16_t selected_weapon_def = 0;
	if (ttapp::client::config().skin_changer.m_selected_weapon >= 0 &&
		ttapp::client::config().skin_changer.m_selected_weapon < static_cast<int>(schema.weapons.size())) {
		selected_weapon_def = schema.weapons[ttapp::client::config().skin_changer.m_selected_weapon].definition_index;
	}

	// Column 2: Paint Kits
	ImGui::BeginChild("##weapon_kits_col", ImVec2(240.0f * scale, 450.0f * scale), false, ImGuiWindowFlags_NoScrollbar);
	g_menu_gui.group_title("Paint Kits");

	// Search Box
	static char weapon_search[64] = "";
	ImGui::SetNextItemWidth(230.0f * scale);
	ImGui::InputTextWithHint("##weapon_search", "Search...", weapon_search, sizeof(weapon_search));

	ImGui::Spacing();

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(5.0f * scale, 5.0f * scale));
	ImGui::BeginChild("##weapon_kits_list", ImVec2(230.0f * scale, 375.0f * scale), true);

	schema.ensure_paint_kits_for_item(selected_weapon_def);
	auto kits_it = schema.item_paint_kits.find(selected_weapon_def);
	if (kits_it != schema.item_paint_kits.end() && !kits_it->second.empty()) {
		auto& kits = kits_it->second;
		const int config_index = Config::skin_changer_t::get_config_index(selected_weapon_def);
		auto& weapon_skin = ttapp::client::config().skin_changer.weapon_skins[config_index];

		std::string search_str = weapon_search;
		std::transform(search_str.begin(), search_str.end(), search_str.begin(), ::tolower);

		for (int i = 0; i < static_cast<int>(kits.size()); ++i) {
			std::string kit_name_lower = kits[i].name;
			std::transform(kit_name_lower.begin(), kit_name_lower.end(), kit_name_lower.begin(), ::tolower);

			if (!search_str.empty() && kit_name_lower.find(search_str) == std::string::npos)
				continue;

			const bool is_selected = weapon_skin.paint_kit == i;

			// Indent slightly to make room for rarity stripe
			ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 6.0f);

			ImGui::PushID(i);
			if (ImGui::Selectable(kits[i].name.c_str(), is_selected)) {
				weapon_skin.paint_kit = i;
				ttapp::client::skin_changer().should_update = true;
			}

			// Draw rarity stripe
			const ImVec2 p_min = ImGui::GetItemRectMin();
			const ImVec2 p_max = ImGui::GetItemRectMax();
			ImGui::GetWindowDrawList()->AddRectFilled(
				ImVec2(p_min.x + 1.0f, p_min.y + 1.0f),
				ImVec2(p_min.x + 4.0f, p_max.y - 1.0f),
				get_rarity_color(kits[i].rarity)
			);

			ImGui::PopID();
		}
	}
	else {
		ImGui::TextDisabled("No skins available");
	}

	ImGui::EndChild();
	ImGui::PopStyleVar();
	ImGui::EndChild();

	ImGui::SameLine(0.0f, 10.0f * scale);

	// Column 3: Preview & Settings
	ImGui::BeginChild("##weapon_preview_col", ImVec2(300.0f * scale, 450.0f * scale), false, ImGuiWindowFlags_NoScrollbar);
	g_menu_gui.group_title("Preview");

	const int config_index = Config::skin_changer_t::get_config_index(selected_weapon_def);
	auto& weapon_skin = ttapp::client::config().skin_changer.weapon_skins[config_index];

	// Get current weapon and paint kit name
	std::string wpn_name = (ttapp::client::config().skin_changer.m_selected_weapon >= 0 &&
		ttapp::client::config().skin_changer.m_selected_weapon < static_cast<int>(schema.weapons.size()))
		? schema.weapons[ttapp::client::config().skin_changer.m_selected_weapon].name
		: "AK-47";
	std::string kit_name = "Default";
	if (kits_it != schema.item_paint_kits.end() && weapon_skin.paint_kit >= 0 && weapon_skin.paint_kit < static_cast<int>(kits_it->second.size())) {
		kit_name = kits_it->second[weapon_skin.paint_kit].name;
	}
	std::string full_skin_name = wpn_name + " | " + kit_name;

	// Preview image placeholder child box
	ImGui::BeginChild("##preview_box", ImVec2(290.0f * scale, 180.0f * scale), true);

	// Placeholder image text/draw list
	ImVec2 p_pos = ImGui::GetWindowPos();
	ImVec2 p_sz = ImGui::GetWindowSize();
	ImGui::GetWindowDrawList()->AddRectFilled(p_pos, ImVec2(p_pos.x + p_sz.x, p_pos.y + p_sz.y), IM_COL32(12, 12, 12, 255), 5.0f * scale);

	CImageProxySource* preview_image = nullptr;
	auto* panorama = ttapp::client::interfaces().m_panorama;
	if (panorama) {
		auto* ui_engine = panorama->AccessUIEngine();
		if (ui_engine) {
			auto* resource_mgr = ui_engine->GetResourceManager();
			if (resource_mgr) {
				if (ttapp::client::config().skin_changer.m_selected_weapon >= 0 &&
					ttapp::client::config().skin_changer.m_selected_weapon < static_cast<int>(schema.weapons.size())) {
					auto& weapon_info = schema.weapons[ttapp::client::config().skin_changer.m_selected_weapon];
					if (kits_it != schema.item_paint_kits.end() && weapon_skin.paint_kit >= 0 && weapon_skin.paint_kit < static_cast<int>(kits_it->second.size())) {
						auto& kit = kits_it->second[weapon_skin.paint_kit];
						if (kit.id == 0 || kit.skinToken.empty() || kit.skinToken == "default" || kit.skinToken == "Default") {
							if (!weapon_info.m_image && !weapon_info.simple_name.empty()) {
								std::string path = "s2r://panorama/images/econ/weapons/base_weapons/" + weapon_info.simple_name + "_png.vtex";
								weapon_info.m_image = resource_mgr->LoadImageInternal(path.c_str(), EImageFormat::RGBA8888);
							}
							preview_image = weapon_info.m_image;
						} else {
							if (!kit.m_image && !weapon_info.simple_name.empty()) {
								std::string path = "s2r://panorama/images/econ/default_generated/" + weapon_info.simple_name + "_" + kit.skinToken + "_light_png.vtex";
								kit.m_image = resource_mgr->LoadImageInternal(path.c_str(), EImageFormat::RGBA8888);
							}
							preview_image = kit.m_image;
						}
					}
				}
			}
		}
	}

	if (preview_image) {
		ID3D11ShaderResourceView* tex = preview_image->GetNativeTexture();
		if (tex) {
			ImVec2 imgSize = preview_image->GetImageSize();
			float availW = p_sz.x - 20.0f * scale;
			float availH = p_sz.y - 50.0f * scale;
			float img_scale = 1.0f;
			if (imgSize.x > 0.f && imgSize.y > 0.f) {
				float scaleW = availW / imgSize.x;
				float scaleH = availH / imgSize.y;
				img_scale = (scaleW < scaleH ? scaleW : scaleH);
			}
			ImVec2 drawSize = ImVec2(imgSize.x * img_scale, imgSize.y * img_scale);
			ImVec2 center = ImVec2(p_pos.x + p_sz.x * 0.5f, p_pos.y + (p_sz.y - 35.0f * scale) * 0.5f);
			ImVec2 imgMin = ImVec2(center.x - drawSize.x * 0.5f, center.y - drawSize.y * 0.5f);
			ImVec2 imgMax = ImVec2(center.x + drawSize.x * 0.5f, center.y + drawSize.y * 0.5f);
			ImGui::GetWindowDrawList()->AddImageRounded(
				(ImTextureID)tex,
				imgMin,
				imgMax,
				ImVec2(0, 0),
				ImVec2(1, 1),
				IM_COL32_WHITE,
				2.0f * scale,
				ImDrawFlags_RoundCornersAll
			);
		} else {
			ImVec2 text_sz = ImGui::CalcTextSize("Loading Preview...");
			ImGui::GetWindowDrawList()->AddText(
				ImVec2(p_pos.x + (p_sz.x - text_sz.x) * 0.5f, p_pos.y + (p_sz.y - text_sz.y) * 0.4f),
				g_menu_gui.text_disabled.to_im_color(),
				"Loading Preview..."
			);
		}
	} else {
		ImVec2 text_sz = ImGui::CalcTextSize("[ Preview Not Loaded ]");
		ImGui::GetWindowDrawList()->AddText(
			ImVec2(p_pos.x + (p_sz.x - text_sz.x) * 0.5f, p_pos.y + (p_sz.y - text_sz.y) * 0.4f),
			g_menu_gui.text_disabled.to_im_color(),
			"[ Preview Not Loaded ]"
		);
	}

	// Underline
	ImGui::GetWindowDrawList()->AddLine(
		ImVec2(p_pos.x + 20.0f * scale, p_pos.y + p_sz.y - 40.0f * scale),
		ImVec2(p_pos.x + p_sz.x - 20.0f * scale, p_pos.y + p_sz.y - 40.0f * scale),
		g_menu_gui.accent_color.to_im_color(),
		2.0f * scale
	);

	// Skin Name
	ImVec2 name_sz = ImGui::CalcTextSize(full_skin_name.c_str());
	ImGui::GetWindowDrawList()->AddText(
		ImVec2(p_pos.x + (p_sz.x - name_sz.x) * 0.5f, p_pos.y + p_sz.y - 30.0f * scale),
		g_menu_gui.text.to_im_color(),
		full_skin_name.c_str()
	);

	ImGui::EndChild();

	ImGui::Spacing();
	ImGui::Text("Settings");
	ImGui::Separator();
	ImGui::Spacing();

	// Settings controls
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4.0f * scale, 3.0f * scale));

	// Wear
	ImGui::AlignTextToFramePadding();
	ImGui::Text("Wear");
	ImGui::SameLine(100.0f * scale);
	ImGui::SetNextItemWidth(180.0f * scale);
	if (ImGui::SliderFloat("##weapon_wear", &weapon_skin.wear, 0.0f, 1.0f, "%.4f")) {
		ttapp::client::skin_changer().should_update = true;
	}

	// Seed
	ImGui::AlignTextToFramePadding();
	ImGui::Text("Seed");
	ImGui::SameLine(100.0f * scale);
	ImGui::SetNextItemWidth(180.0f * scale);
	if (ImGui::InputInt("##weapon_seed", &weapon_skin.seed, 0, 0)) {
		ttapp::client::skin_changer().should_update = true;
	}

	// StatTrak
	ImGui::AlignTextToFramePadding();
	ImGui::Text("StatTrak");
	ImGui::SameLine(100.0f * scale);
	ImGui::SetNextItemWidth(180.0f * scale);
	if (ImGui::Checkbox("##weapon_stattrak", &weapon_skin.stattrak_enabled)) {
		ttapp::client::skin_changer().should_update = true;
	}
	if (weapon_skin.stattrak_enabled) {
		ImGui::AlignTextToFramePadding();
		ImGui::Text("Count");
		ImGui::SameLine(100.0f * scale);
		ImGui::SetNextItemWidth(180.0f * scale);
		if (ImGui::InputInt("##weapon_stattrak_count", &weapon_skin.stattrak_count, 0, 0)) {
			if (weapon_skin.stattrak_count < 0)
				weapon_skin.stattrak_count = 0;
			ttapp::client::skin_changer().should_update = true;
		}
	}

	if (draw_color4_controls("weapon_custom_color", weapon_skin.custom_colors_enabled, weapon_skin.custom_colors, scale))
		ttapp::client::skin_changer().should_update = true;

	if (draw_sticker_editor(weapon_skin, scale))
		ttapp::client::skin_changer().should_update = true;

	if (draw_keychain_editor(weapon_skin, scale))
		ttapp::client::skin_changer().should_update = true;

	// Custom Name
	ImGui::AlignTextToFramePadding();
	ImGui::Text("Custom Name");
	ImGui::SameLine(100.0f * scale);
	ImGui::SetNextItemWidth(180.0f * scale);
	if (ImGui::InputText("##weapon_name", weapon_skin.custom_name, sizeof(weapon_skin.custom_name))) {
		ttapp::client::skin_changer().should_update = true;
	}

	ImGui::PopStyleVar();

	ImGui::EndChild();
}

static void draw_skins_knives_tab(float scale) {
	auto& schema = ttapp::client::item_schema();
	if (!schema.is_initialized()) {
		ImGui::TextDisabled("Loading knife schema...");
		return;
	}

	// Column 1: Models
	ImGui::BeginChild("##knife_models_col", ImVec2(200.0f * scale, 450.0f * scale), false, ImGuiWindowFlags_NoScrollbar);
	g_menu_gui.group_title("Models");
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(5.0f * scale, 5.0f * scale));
	ImGui::BeginChild("##knife_models_list", ImVec2(190.0f * scale, 410.0f * scale), true);
	int selected_knife_idx = ttapp::client::config().knife_changer.m_knife;
	bool knife_model_changed = false;
	for (int i = 0; i < static_cast<int>(schema.knives.size()); ++i) {
		const bool is_selected = selected_knife_idx == i;
		ImGui::PushID(i);
		if (ImGui::Selectable(schema.knives[i].name.c_str(), is_selected)) {
			ttapp::client::config().knife_changer.m_knife = i;
			knife_model_changed = true;
		}
		ImGui::PopID();
	}
	ImGui::EndChild();
	ImGui::PopStyleVar();
	ImGui::EndChild();

	ImGui::SameLine(0.0f, 10.0f * scale);

	uint16_t selected_knife = 0;
	if (ttapp::client::config().knife_changer.m_knife >= 0 &&
		ttapp::client::config().knife_changer.m_knife < static_cast<int>(schema.knives.size())) {
		selected_knife = schema.knives[ttapp::client::config().knife_changer.m_knife].definition_index;
	}

	if (knife_model_changed) {
		ttapp::client::config().knife_changer.m_paint_kit = 0;
		ttapp::client::skin_changer().should_update = true;
	}

	// Column 2: Paint Kits
	ImGui::BeginChild("##knife_kits_col", ImVec2(240.0f * scale, 450.0f * scale), false, ImGuiWindowFlags_NoScrollbar);
	g_menu_gui.group_title("Paint Kits");

	static char knife_search[64] = "";
	ImGui::SetNextItemWidth(230.0f * scale);
	ImGui::InputTextWithHint("##knife_search", "Search...", knife_search, sizeof(knife_search));
	ImGui::Spacing();

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(5.0f * scale, 5.0f * scale));
	ImGui::BeginChild("##knife_kits_list", ImVec2(230.0f * scale, 375.0f * scale), true);

	schema.ensure_paint_kits_for_item(selected_knife);
	auto kits_it = schema.item_paint_kits.find(selected_knife);
	if (kits_it != schema.item_paint_kits.end() && !kits_it->second.empty()) {
		auto& kits = kits_it->second;
		std::string search_str = knife_search;
		std::transform(search_str.begin(), search_str.end(), search_str.begin(), ::tolower);

		for (int i = 0; i < static_cast<int>(kits.size()); ++i) {
			std::string kit_name_lower = kits[i].name;
			std::transform(kit_name_lower.begin(), kit_name_lower.end(), kit_name_lower.begin(), ::tolower);

			if (!search_str.empty() && kit_name_lower.find(search_str) == std::string::npos)
				continue;

			const bool is_selected = ttapp::client::config().knife_changer.m_paint_kit == i;

			ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 6.0f);
			ImGui::PushID(i);
			if (ImGui::Selectable(kits[i].name.c_str(), is_selected)) {
				ttapp::client::config().knife_changer.m_paint_kit = i;
				ttapp::client::skin_changer().should_update = true;
			}

			const ImVec2 p_min = ImGui::GetItemRectMin();
			const ImVec2 p_max = ImGui::GetItemRectMax();
			ImGui::GetWindowDrawList()->AddRectFilled(
				ImVec2(p_min.x + 1.0f, p_min.y + 1.0f),
				ImVec2(p_min.x + 4.0f, p_max.y - 1.0f),
				get_rarity_color(kits[i].rarity)
			);
			ImGui::PopID();
		}
	}
	else {
		ImGui::TextDisabled("No skins available");
	}
	ImGui::EndChild();
	ImGui::PopStyleVar();
	ImGui::EndChild();

	ImGui::SameLine(0.0f, 10.0f * scale);

	// Column 3: Preview & Settings
	ImGui::BeginChild("##knife_preview_col", ImVec2(300.0f * scale, 450.0f * scale), false, ImGuiWindowFlags_NoScrollbar);
	g_menu_gui.group_title("Preview");

	std::string wpn_name = (ttapp::client::config().knife_changer.m_knife >= 0 &&
		ttapp::client::config().knife_changer.m_knife < static_cast<int>(schema.knives.size()))
		? schema.knives[ttapp::client::config().knife_changer.m_knife].name
		: "Knife";
	std::string kit_name = "Default";
	if (kits_it != schema.item_paint_kits.end() && ttapp::client::config().knife_changer.m_paint_kit >= 0 && ttapp::client::config().knife_changer.m_paint_kit < static_cast<int>(kits_it->second.size())) {
		kit_name = kits_it->second[ttapp::client::config().knife_changer.m_paint_kit].name;
	}
	std::string full_skin_name = wpn_name + " | " + kit_name;

	ImGui::BeginChild("##knife_preview_box", ImVec2(290.0f * scale, 180.0f * scale), true);
	ImVec2 p_pos = ImGui::GetWindowPos();
	ImVec2 p_sz = ImGui::GetWindowSize();
	ImGui::GetWindowDrawList()->AddRectFilled(p_pos, ImVec2(p_pos.x + p_sz.x, p_pos.y + p_sz.y), IM_COL32(12, 12, 12, 255), 5.0f * scale);

	CImageProxySource* preview_image = nullptr;
	auto* panorama = ttapp::client::interfaces().m_panorama;
	if (panorama) {
		auto* ui_engine = panorama->AccessUIEngine();
		if (ui_engine) {
			auto* resource_mgr = ui_engine->GetResourceManager();
			if (resource_mgr) {
				if (ttapp::client::config().knife_changer.m_knife >= 0 &&
					ttapp::client::config().knife_changer.m_knife < static_cast<int>(schema.knives.size())) {
					auto& knife_info = schema.knives[ttapp::client::config().knife_changer.m_knife];
					if (kits_it != schema.item_paint_kits.end() && ttapp::client::config().knife_changer.m_paint_kit >= 0 && ttapp::client::config().knife_changer.m_paint_kit < static_cast<int>(kits_it->second.size())) {
						auto& kit = kits_it->second[ttapp::client::config().knife_changer.m_paint_kit];
						if (kit.id == 0 || kit.skinToken.empty() || kit.skinToken == "default" || kit.skinToken == "Default") {
							if (!knife_info.m_image && !knife_info.simple_name.empty()) {
								std::string path = "s2r://panorama/images/econ/weapons/base_weapons/" + knife_info.simple_name + "_png.vtex";
								knife_info.m_image = resource_mgr->LoadImageInternal(path.c_str(), EImageFormat::RGBA8888);
							}
							preview_image = knife_info.m_image;
						} else {
							if (!kit.m_image && !knife_info.simple_name.empty()) {
								std::string path = "s2r://panorama/images/econ/default_generated/" + knife_info.simple_name + "_" + kit.skinToken + "_light_png.vtex";
								kit.m_image = resource_mgr->LoadImageInternal(path.c_str(), EImageFormat::RGBA8888);
							}
							preview_image = kit.m_image;
						}
					}
				}
			}
		}
	}

	if (preview_image) {
		ID3D11ShaderResourceView* tex = preview_image->GetNativeTexture();
		if (tex) {
			ImVec2 imgSize = preview_image->GetImageSize();
			float availW = p_sz.x - 20.0f * scale;
			float availH = p_sz.y - 50.0f * scale;
			float img_scale = 1.0f;
			if (imgSize.x > 0.f && imgSize.y > 0.f) {
				float scaleW = availW / imgSize.x;
				float scaleH = availH / imgSize.y;
				img_scale = (scaleW < scaleH ? scaleW : scaleH);
			}
			ImVec2 drawSize = ImVec2(imgSize.x * img_scale, imgSize.y * img_scale);
			ImVec2 center = ImVec2(p_pos.x + p_sz.x * 0.5f, p_pos.y + (p_sz.y - 35.0f * scale) * 0.5f);
			ImVec2 imgMin = ImVec2(center.x - drawSize.x * 0.5f, center.y - drawSize.y * 0.5f);
			ImVec2 imgMax = ImVec2(center.x + drawSize.x * 0.5f, center.y + drawSize.y * 0.5f);
			ImGui::GetWindowDrawList()->AddImageRounded(
				(ImTextureID)tex,
				imgMin,
				imgMax,
				ImVec2(0, 0),
				ImVec2(1, 1),
				IM_COL32_WHITE,
				2.0f * scale,
				ImDrawFlags_RoundCornersAll
			);
		} else {
			ImVec2 text_sz = ImGui::CalcTextSize("Loading Preview...");
			ImGui::GetWindowDrawList()->AddText(
				ImVec2(p_pos.x + (p_sz.x - text_sz.x) * 0.5f, p_pos.y + (p_sz.y - text_sz.y) * 0.4f),
				g_menu_gui.text_disabled.to_im_color(),
				"Loading Preview..."
			);
		}
	} else {
		ImVec2 text_sz = ImGui::CalcTextSize("[ Preview Not Loaded ]");
		ImGui::GetWindowDrawList()->AddText(
			ImVec2(p_pos.x + (p_sz.x - text_sz.x) * 0.5f, p_pos.y + (p_sz.y - text_sz.y) * 0.4f),
			g_menu_gui.text_disabled.to_im_color(),
			"[ Preview Not Loaded ]"
		);
	}

	ImGui::GetWindowDrawList()->AddLine(
		ImVec2(p_pos.x + 20.0f * scale, p_pos.y + p_sz.y - 40.0f * scale),
		ImVec2(p_pos.x + p_sz.x - 20.0f * scale, p_pos.y + p_sz.y - 40.0f * scale),
		g_menu_gui.accent_color.to_im_color(),
		2.0f * scale
	);

	ImVec2 name_sz = ImGui::CalcTextSize(full_skin_name.c_str());
	ImGui::GetWindowDrawList()->AddText(
		ImVec2(p_pos.x + (p_sz.x - name_sz.x) * 0.5f, p_pos.y + p_sz.y - 30.0f * scale),
		g_menu_gui.text.to_im_color(),
		full_skin_name.c_str()
	);
	ImGui::EndChild();

	ImGui::Spacing();
	ImGui::Text("Settings");
	ImGui::Separator();
	ImGui::Spacing();

	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4.0f * scale, 3.0f * scale));

	// Wear
	ImGui::AlignTextToFramePadding();
	ImGui::Text("Wear");
	ImGui::SameLine(100.0f * scale);
	ImGui::SetNextItemWidth(180.0f * scale);
	if (ImGui::SliderFloat("##knife_wear", &ttapp::client::config().knife_changer.m_wear, 0.0f, 1.0f, "%.4f")) {
		ttapp::client::skin_changer().should_update = true;
	}

	// Seed
	ImGui::AlignTextToFramePadding();
	ImGui::Text("Seed");
	ImGui::SameLine(100.0f * scale);
	ImGui::SetNextItemWidth(180.0f * scale);
	if (ImGui::InputInt("##knife_seed", &ttapp::client::config().knife_changer.m_seed, 0, 0)) {
		ttapp::client::skin_changer().should_update = true;
	}

	// StatTrak
	ImGui::AlignTextToFramePadding();
	ImGui::Text("StatTrak");
	ImGui::SameLine(100.0f * scale);
	ImGui::SetNextItemWidth(180.0f * scale);
	if (ImGui::Checkbox("##knife_stattrak", &ttapp::client::config().knife_changer.m_stattrak_enabled)) {
		ttapp::client::skin_changer().should_update = true;
	}
	if (ttapp::client::config().knife_changer.m_stattrak_enabled) {
		ImGui::AlignTextToFramePadding();
		ImGui::Text("Count");
		ImGui::SameLine(100.0f * scale);
		ImGui::SetNextItemWidth(180.0f * scale);
		if (ImGui::InputInt("##knife_stattrak_count", &ttapp::client::config().knife_changer.m_stattrak_count, 0, 0)) {
			if (ttapp::client::config().knife_changer.m_stattrak_count < 0)
				ttapp::client::config().knife_changer.m_stattrak_count = 0;
			ttapp::client::skin_changer().should_update = true;
		}
	}

	if (draw_color4_controls("knife_custom_color", ttapp::client::config().knife_changer.m_custom_colors_enabled,
		ttapp::client::config().knife_changer.m_custom_colors, scale))
		ttapp::client::skin_changer().should_update = true;

	// Custom Name
	ImGui::AlignTextToFramePadding();
	ImGui::Text("Custom Name");
	ImGui::SameLine(100.0f * scale);
	ImGui::SetNextItemWidth(180.0f * scale);
	if (ImGui::InputText("##knife_name", ttapp::client::config().knife_changer.m_custom_name, sizeof(ttapp::client::config().knife_changer.m_custom_name))) {
		ttapp::client::skin_changer().should_update = true;
	}

	ImGui::PopStyleVar();
	ImGui::EndChild();
}

static void draw_skins_gloves_tab(float scale) {
	auto& schema = ttapp::client::item_schema();
	if (!schema.is_initialized()) {
		ImGui::TextDisabled("Loading glove schema...");
		return;
	}

	// Column 1: Models
	ImGui::BeginChild("##glove_models_col", ImVec2(200.0f * scale, 450.0f * scale), false, ImGuiWindowFlags_NoScrollbar);
	g_menu_gui.group_title("Models");
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(5.0f * scale, 5.0f * scale));
	ImGui::BeginChild("##glove_models_list", ImVec2(190.0f * scale, 410.0f * scale), true);
	int selected_glove_idx = ttapp::client::config().glove_changer.m_glove;
	bool glove_model_changed = false;
	for (int i = 0; i < static_cast<int>(schema.gloves.size()); ++i) {
		const bool is_selected = selected_glove_idx == i;
		ImGui::PushID(i);
		if (ImGui::Selectable(schema.gloves[i].name.c_str(), is_selected)) {
			ttapp::client::config().glove_changer.m_glove = i;
			glove_model_changed = true;
		}
		ImGui::PopID();
	}
	ImGui::EndChild();
	ImGui::PopStyleVar();
	ImGui::EndChild();

	ImGui::SameLine(0.0f, 10.0f * scale);

	uint16_t selected_glove = 0;
	if (ttapp::client::config().glove_changer.m_glove >= 0 &&
		ttapp::client::config().glove_changer.m_glove < static_cast<int>(schema.gloves.size())) {
		selected_glove = schema.gloves[ttapp::client::config().glove_changer.m_glove].definition_index;
	}

	if (glove_model_changed) {
		auto& glove_skins = schema.get_paint_kit_names_for_item(selected_glove);
		ttapp::client::config().glove_changer.m_paint_kit = glove_skins.size() > 1 ? 1 : 0;
		ttapp::client::glove_changer().should_update = true;
	}

	// Column 2: Paint Kits
	ImGui::BeginChild("##glove_kits_col", ImVec2(240.0f * scale, 450.0f * scale), false, ImGuiWindowFlags_NoScrollbar);
	g_menu_gui.group_title("Paint Kits");

	static char glove_search[64] = "";
	ImGui::SetNextItemWidth(230.0f * scale);
	ImGui::InputTextWithHint("##glove_search", "Search...", glove_search, sizeof(glove_search));
	ImGui::Spacing();

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(5.0f * scale, 5.0f * scale));
	ImGui::BeginChild("##glove_kits_list", ImVec2(230.0f * scale, 375.0f * scale), true);

	schema.ensure_paint_kits_for_item(selected_glove);
	auto kits_it = schema.item_paint_kits.find(selected_glove);
	if (kits_it != schema.item_paint_kits.end() && !kits_it->second.empty()) {
		auto& kits = kits_it->second;
		std::string search_str = glove_search;
		std::transform(search_str.begin(), search_str.end(), search_str.begin(), ::tolower);

		for (int i = 0; i < static_cast<int>(kits.size()); ++i) {
			std::string kit_name_lower = kits[i].name;
			std::transform(kit_name_lower.begin(), kit_name_lower.end(), kit_name_lower.begin(), ::tolower);

			if (!search_str.empty() && kit_name_lower.find(search_str) == std::string::npos)
				continue;

			const bool is_selected = ttapp::client::config().glove_changer.m_paint_kit == i;

			ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 6.0f);
			ImGui::PushID(i);
			if (ImGui::Selectable(kits[i].name.c_str(), is_selected)) {
				ttapp::client::config().glove_changer.m_paint_kit = i;
				ttapp::client::glove_changer().should_update = true;
			}

			const ImVec2 p_min = ImGui::GetItemRectMin();
			const ImVec2 p_max = ImGui::GetItemRectMax();
			ImGui::GetWindowDrawList()->AddRectFilled(
				ImVec2(p_min.x + 1.0f, p_min.y + 1.0f),
				ImVec2(p_min.x + 4.0f, p_max.y - 1.0f),
				get_rarity_color(kits[i].rarity)
			);
			ImGui::PopID();
		}
	}
	else {
		ImGui::TextDisabled("No skins available");
	}
	ImGui::EndChild();
	ImGui::PopStyleVar();
	ImGui::EndChild();

	ImGui::SameLine(0.0f, 10.0f * scale);

	// Column 3: Preview & Settings
	ImGui::BeginChild("##glove_preview_col", ImVec2(300.0f * scale, 450.0f * scale), false, ImGuiWindowFlags_NoScrollbar);
	g_menu_gui.group_title("Preview");

	std::string wpn_name = (ttapp::client::config().glove_changer.m_glove >= 0 &&
		ttapp::client::config().glove_changer.m_glove < static_cast<int>(schema.gloves.size()))
		? schema.gloves[ttapp::client::config().glove_changer.m_glove].name
		: "Glove";
	std::string kit_name = "Default";
	if (kits_it != schema.item_paint_kits.end() && ttapp::client::config().glove_changer.m_paint_kit >= 0 && ttapp::client::config().glove_changer.m_paint_kit < static_cast<int>(kits_it->second.size())) {
		kit_name = kits_it->second[ttapp::client::config().glove_changer.m_paint_kit].name;
	}
	std::string full_skin_name = wpn_name + " | " + kit_name;

	ImGui::BeginChild("##glove_preview_box", ImVec2(290.0f * scale, 180.0f * scale), true);
	ImVec2 p_pos = ImGui::GetWindowPos();
	ImVec2 p_sz = ImGui::GetWindowSize();
	ImGui::GetWindowDrawList()->AddRectFilled(p_pos, ImVec2(p_pos.x + p_sz.x, p_pos.y + p_sz.y), IM_COL32(12, 12, 12, 255), 5.0f * scale);

	CImageProxySource* preview_image = nullptr;
	auto* panorama = ttapp::client::interfaces().m_panorama;
	if (panorama) {
		auto* ui_engine = panorama->AccessUIEngine();
		if (ui_engine) {
			auto* resource_mgr = ui_engine->GetResourceManager();
			if (resource_mgr) {
				if (ttapp::client::config().glove_changer.m_glove >= 0 &&
					ttapp::client::config().glove_changer.m_glove < static_cast<int>(schema.gloves.size())) {
					auto& glove_info = schema.gloves[ttapp::client::config().glove_changer.m_glove];
					if (kits_it != schema.item_paint_kits.end() && ttapp::client::config().glove_changer.m_paint_kit >= 0 && ttapp::client::config().glove_changer.m_paint_kit < static_cast<int>(kits_it->second.size())) {
						auto& kit = kits_it->second[ttapp::client::config().glove_changer.m_paint_kit];
						if (kit.id == 0 || kit.skinToken.empty() || kit.skinToken == "default" || kit.skinToken == "Default") {
							if (!glove_info.m_image && !glove_info.simple_name.empty()) {
								std::string path = "s2r://panorama/images/econ/weapons/base_weapons/" + glove_info.simple_name + "_png.vtex";
								glove_info.m_image = resource_mgr->LoadImageInternal(path.c_str(), EImageFormat::RGBA8888);
							}
							preview_image = glove_info.m_image;
						} else {
							if (!kit.m_image && !glove_info.simple_name.empty()) {
								std::string path = "s2r://panorama/images/econ/default_generated/" + glove_info.simple_name + "_" + kit.skinToken + "_light_png.vtex";
								kit.m_image = resource_mgr->LoadImageInternal(path.c_str(), EImageFormat::RGBA8888);
							}
							preview_image = kit.m_image;
						}
					}
				}
			}
		}
	}

	if (preview_image) {
		ID3D11ShaderResourceView* tex = preview_image->GetNativeTexture();
		if (tex) {
			ImVec2 imgSize = preview_image->GetImageSize();
			float availW = p_sz.x - 20.0f * scale;
			float availH = p_sz.y - 50.0f * scale;
			float img_scale = 1.0f;
			if (imgSize.x > 0.f && imgSize.y > 0.f) {
				float scaleW = availW / imgSize.x;
				float scaleH = availH / imgSize.y;
				img_scale = (scaleW < scaleH ? scaleW : scaleH);
			}
			ImVec2 drawSize = ImVec2(imgSize.x * img_scale, imgSize.y * img_scale);
			ImVec2 center = ImVec2(p_pos.x + p_sz.x * 0.5f, p_pos.y + (p_sz.y - 35.0f * scale) * 0.5f);
			ImVec2 imgMin = ImVec2(center.x - drawSize.x * 0.5f, center.y - drawSize.y * 0.5f);
			ImVec2 imgMax = ImVec2(center.x + drawSize.x * 0.5f, center.y + drawSize.y * 0.5f);
			ImGui::GetWindowDrawList()->AddImageRounded(
				(ImTextureID)tex,
				imgMin,
				imgMax,
				ImVec2(0, 0),
				ImVec2(1, 1),
				IM_COL32_WHITE,
				2.0f * scale,
				ImDrawFlags_RoundCornersAll
			);
		} else {
			ImVec2 text_sz = ImGui::CalcTextSize("Loading Preview...");
			ImGui::GetWindowDrawList()->AddText(
				ImVec2(p_pos.x + (p_sz.x - text_sz.x) * 0.5f, p_pos.y + (p_sz.y - text_sz.y) * 0.4f),
				g_menu_gui.text_disabled.to_im_color(),
				"Loading Preview..."
			);
		}
	} else {
		ImVec2 text_sz = ImGui::CalcTextSize("[ Preview Not Loaded ]");
		ImGui::GetWindowDrawList()->AddText(
			ImVec2(p_pos.x + (p_sz.x - text_sz.x) * 0.5f, p_pos.y + (p_sz.y - text_sz.y) * 0.4f),
			g_menu_gui.text_disabled.to_im_color(),
			"[ Preview Not Loaded ]"
		);
	}

	ImGui::GetWindowDrawList()->AddLine(
		ImVec2(p_pos.x + 20.0f * scale, p_pos.y + p_sz.y - 40.0f * scale),
		ImVec2(p_pos.x + p_sz.x - 20.0f * scale, p_pos.y + p_sz.y - 40.0f * scale),
		g_menu_gui.accent_color.to_im_color(),
		2.0f * scale
	);

	ImVec2 name_sz = ImGui::CalcTextSize(full_skin_name.c_str());
	ImGui::GetWindowDrawList()->AddText(
		ImVec2(p_pos.x + (p_sz.x - name_sz.x) * 0.5f, p_pos.y + p_sz.y - 30.0f * scale),
		g_menu_gui.text.to_im_color(),
		full_skin_name.c_str()
	);
	ImGui::EndChild();

	ImGui::Spacing();
	ImGui::Text("Settings");
	ImGui::Separator();
	ImGui::Spacing();

	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4.0f * scale, 3.0f * scale));

	// Wear
	ImGui::AlignTextToFramePadding();
	ImGui::Text("Wear");
	ImGui::SameLine(100.0f * scale);
	ImGui::SetNextItemWidth(180.0f * scale);
	if (ImGui::SliderFloat("##glove_wear", &ttapp::client::config().glove_changer.m_wear, 0.0f, 1.0f, "%.4f")) {
		ttapp::client::glove_changer().should_update = true;
	}

	// Seed
	ImGui::AlignTextToFramePadding();
	ImGui::Text("Seed");
	ImGui::SameLine(100.0f * scale);
	ImGui::SetNextItemWidth(180.0f * scale);
	if (ImGui::InputInt("##glove_seed", &ttapp::client::config().glove_changer.m_seed, 0, 0)) {
		ttapp::client::glove_changer().should_update = true;
	}

	ImGui::PopStyleVar();
	ImGui::EndChild();
}

static void draw_skins_agent_tab(float scale) {
	auto& schema = ttapp::client::item_schema();
	if (!schema.is_initialized()) {
		ImGui::TextDisabled("Loading agent schema...");
		return;
	}

	// Column 1: Factions
	ImGui::BeginChild("##agent_factions_col", ImVec2(200.0f * scale, 450.0f * scale), false, ImGuiWindowFlags_NoScrollbar);
	g_menu_gui.group_title("Faction");
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(5.0f * scale, 5.0f * scale));
	ImGui::BeginChild("##agent_factions_list", ImVec2(190.0f * scale, 410.0f * scale), true);

	static int selected_faction = 0; // 0: CT, 1: T
	if (ImGui::Selectable("Counter-Terrorists (CT)", selected_faction == 0)) {
		selected_faction = 0;
	}
	ImGui::Spacing();
	if (ImGui::Selectable("Terrorists (T)", selected_faction == 1)) {
		selected_faction = 1;
	}

	ImGui::EndChild();
	ImGui::PopStyleVar();
	ImGui::EndChild();

	ImGui::SameLine(0.0f, 10.0f * scale);

	// Column 2: Agent Models
	ImGui::BeginChild("##agent_models_col", ImVec2(240.0f * scale, 450.0f * scale), false, ImGuiWindowFlags_NoScrollbar);
	g_menu_gui.group_title("Agents");

	static char agent_search[64] = "";
	ImGui::SetNextItemWidth(230.0f * scale);
	ImGui::InputTextWithHint("##agent_search", "Search...", agent_search, sizeof(agent_search));
	ImGui::Spacing();

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(5.0f * scale, 5.0f * scale));
	ImGui::BeginChild("##agent_models_list", ImVec2(230.0f * scale, 375.0f * scale), true);

	auto& agents = (selected_faction == 0) ? schema.agents_ct : schema.agents_t;
	int& selected_agent_idx = (selected_faction == 0) ? ttapp::client::config().agent_changer.m_agent_ct : ttapp::client::config().agent_changer.m_agent_t;

	std::string search_str = agent_search;
	std::transform(search_str.begin(), search_str.end(), search_str.begin(), ::tolower);

	for (int i = 0; i < static_cast<int>(agents.size()); ++i) {
		std::string agent_name_lower = agents[i].name;
		std::transform(agent_name_lower.begin(), agent_name_lower.end(), agent_name_lower.begin(), ::tolower);

		if (!search_str.empty() && agent_name_lower.find(search_str) == std::string::npos)
			continue;

		const bool is_selected = selected_agent_idx == i;

		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 6.0f);
		ImGui::PushID(i);
		if (ImGui::Selectable(agents[i].name.c_str(), is_selected)) {
			selected_agent_idx = i;
			ttapp::client::agent_changer().should_update = true;
		}

		const ImVec2 p_min = ImGui::GetItemRectMin();
		const ImVec2 p_max = ImGui::GetItemRectMax();
		ImGui::GetWindowDrawList()->AddRectFilled(
			ImVec2(p_min.x + 1.0f, p_min.y + 1.0f),
			ImVec2(p_min.x + 4.0f, p_max.y - 1.0f),
			get_rarity_color(agents[i].rarity)
		);
		ImGui::PopID();
	}

	ImGui::EndChild();
	ImGui::PopStyleVar();
	ImGui::EndChild();

	ImGui::SameLine(0.0f, 10.0f * scale);

	// Column 3: Preview
	ImGui::BeginChild("##agent_preview_col", ImVec2(300.0f * scale, 450.0f * scale), false, ImGuiWindowFlags_NoScrollbar);
	g_menu_gui.group_title("Preview");

	std::string full_agent_name = "Default Agent";
	if (selected_agent_idx >= 0 && selected_agent_idx < static_cast<int>(agents.size())) {
		full_agent_name = agents[selected_agent_idx].name;
	}

	ImGui::BeginChild("##agent_preview_box", ImVec2(290.0f * scale, 180.0f * scale), true);
	ImVec2 p_pos = ImGui::GetWindowPos();
	ImVec2 p_sz = ImGui::GetWindowSize();
	ImGui::GetWindowDrawList()->AddRectFilled(p_pos, ImVec2(p_pos.x + p_sz.x, p_pos.y + p_sz.y), IM_COL32(12, 12, 12, 255), 5.0f * scale);

	CImageProxySource* preview_image = nullptr;
	auto* panorama = ttapp::client::interfaces().m_panorama;
	if (panorama) {
		auto* ui_engine = panorama->AccessUIEngine();
		if (ui_engine) {
			auto* resource_mgr = ui_engine->GetResourceManager();
			if (resource_mgr) {
				if (selected_agent_idx >= 0 && selected_agent_idx < static_cast<int>(agents.size())) {
					auto& agent_info = agents[selected_agent_idx];
					if (agent_info.definition_index != 0 && !agent_info.simple_name.empty()) {
						if (!agent_info.m_image) {
							std::string path = "s2r://panorama/images/econ/characters/" + agent_info.simple_name + "_png.vtex";
							agent_info.m_image = resource_mgr->LoadImageInternal(path.c_str(), EImageFormat::RGBA8888);
						}
						preview_image = agent_info.m_image;
					}
				}
			}
		}
	}

	if (preview_image) {
		ID3D11ShaderResourceView* tex = preview_image->GetNativeTexture();
		if (tex) {
			ImVec2 imgSize = preview_image->GetImageSize();
			float availW = p_sz.x - 20.0f * scale;
			float availH = p_sz.y - 50.0f * scale;
			float img_scale = 1.0f;
			if (imgSize.x > 0.f && imgSize.y > 0.f) {
				float scaleW = availW / imgSize.x;
				float scaleH = availH / imgSize.y;
				img_scale = (scaleW < scaleH ? scaleW : scaleH);
			}
			ImVec2 drawSize = ImVec2(imgSize.x * img_scale, imgSize.y * img_scale);
			ImVec2 center = ImVec2(p_pos.x + p_sz.x * 0.5f, p_pos.y + (p_sz.y - 35.0f * scale) * 0.5f);
			ImVec2 imgMin = ImVec2(center.x - drawSize.x * 0.5f, center.y - drawSize.y * 0.5f);
			ImVec2 imgMax = ImVec2(center.x + drawSize.x * 0.5f, center.y + drawSize.y * 0.5f);
			ImGui::GetWindowDrawList()->AddImageRounded(
				(ImTextureID)tex,
				imgMin,
				imgMax,
				ImVec2(0, 0),
				ImVec2(1, 1),
				IM_COL32_WHITE,
				2.0f * scale,
				ImDrawFlags_RoundCornersAll
			);
		} else {
			ImVec2 text_sz = ImGui::CalcTextSize("Loading Preview...");
			ImGui::GetWindowDrawList()->AddText(
				ImVec2(p_pos.x + (p_sz.x - text_sz.x) * 0.5f, p_pos.y + (p_sz.y - text_sz.y) * 0.4f),
				g_menu_gui.text_disabled.to_im_color(),
				"Loading Preview..."
			);
		}
	} else {
		ImVec2 text_sz = ImGui::CalcTextSize("[ Preview Not Loaded ]");
		ImGui::GetWindowDrawList()->AddText(
			ImVec2(p_pos.x + (p_sz.x - text_sz.x) * 0.5f, p_pos.y + (p_sz.y - text_sz.y) * 0.4f),
			g_menu_gui.text_disabled.to_im_color(),
			"[ Preview Not Loaded ]"
		);
	}

	ImGui::GetWindowDrawList()->AddLine(
		ImVec2(p_pos.x + 20.0f * scale, p_pos.y + p_sz.y - 40.0f * scale),
		ImVec2(p_pos.x + p_sz.x - 20.0f * scale, p_pos.y + p_sz.y - 40.0f * scale),
		g_menu_gui.accent_color.to_im_color(),
		2.0f * scale
	);

	ImVec2 name_sz = ImGui::CalcTextSize(full_agent_name.c_str());
	ImGui::GetWindowDrawList()->AddText(
		ImVec2(p_pos.x + (p_sz.x - name_sz.x) * 0.5f, p_pos.y + p_sz.y - 30.0f * scale),
		g_menu_gui.text.to_im_color(),
		full_agent_name.c_str()
	);
	ImGui::EndChild();

	ImGui::Spacing();
	ImGui::Text("Settings");
	ImGui::Separator();
	ImGui::Spacing();

	ImGui::TextDisabled("No customize settings for agents.");

	ImGui::EndChild();
}

namespace menu_tabs {

void draw_skins(int selected_skins_tab, float scale)
{
    switch (selected_skins_tab) {
    case static_cast<int>(skins_sidebar_t::general):
        draw_skins_general_tab(scale);
        break;
    case static_cast<int>(skins_sidebar_t::weapons):
        draw_skins_weapons_tab(scale);
        break;
    case static_cast<int>(skins_sidebar_t::knives):
        draw_skins_knives_tab(scale);
        break;
    case static_cast<int>(skins_sidebar_t::gloves):
        draw_skins_gloves_tab(scale);
        break;
    case static_cast<int>(skins_sidebar_t::agent):
        draw_skins_agent_tab(scale);
        break;
    default:
        break;
    }
}

} // namespace menu_tabs
