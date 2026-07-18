#include "../shell/shell.hpp"
#include "../styling.hpp"
#include "gui/assets/fonts.hpp"
#include "imgui.h"
#include "imgui_internal.h"

#include <algorithm>

namespace ksd
{
	bool ListBox(const char* , int* selected, const char* const* items, int count) noexcept
	{
		if (count <= 0)
			return false;

		*selected = std::clamp(*selected, 0, count - 1);

		const float w = ContentWidth();
		const float boxH = static_cast<float>(count) * 20.f + 2.f;
		const ImVec2 pos = CursorPos() + ImVec2(20, 0);
		const ImRect box(pos, pos + ImVec2(w, boxH));

		ImDrawList* dl = ImGui::GetWindowDrawList();
		dl->AddRectFilled(box.Min + ImVec2(1, 1), box.Max - ImVec2(1, 1), style::glass(17, 17, 18, 0.18f));
		dl->AddRect(box.Min, box.Max, style::col::border());

		ImGui::PushClipRect(box.Min + ImVec2(1, 1), box.Max - ImVec2(1, 1), true);

		bool changed = false;
		for (int i = 0; i < count; ++i)
		{
			const ImVec2 rowPos = box.Min + ImVec2(1, 1 + i * 20.f);
			const ImRect row(rowPos, rowPos + ImVec2(w - 2, 18));
			const bool rowSelected = *selected == i;
			const bool hovered = row.Contains(ImGui::GetMousePos());
			const bool pressed = hovered && ImGui::IsMouseReleased(ImGuiMouseButton_Left) && PopUpsAreClosed();

			if (hovered || rowSelected)
				dl->AddRectFilled(row.Min, row.Max, style::glass(40, 40, 40, style::glassWidget));

			const ImU32 textCol = rowSelected ? style::accentU32() : style::col::text();
			const ImVec2 textSize = fonts::CalcUiTextSize(items[i]);
			dl->AddText(
				fonts::Verdana(),
				fonts::kUiSize,
				row.Min + ImVec2(10, (row.GetHeight() - textSize.y) * 0.5f),
				textCol,
				items[i]);

			if (pressed && !rowSelected)
			{
				*selected = i;
				changed = true;
			}
		}

		ImGui::PopClipRect();

		AddItem(ImVec2(w + 20.f, boxH), true);
		return changed;
	}
}
