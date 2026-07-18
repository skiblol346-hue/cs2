#include "UI.hpp"

void CustomUI::Checkbox(const char* label, bool* v)
{
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems) return;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(label);
    const ImVec2 label_size = ImGui::CalcTextSize(label, NULL, true);

    const float square_sz = 16.0f;
    const ImVec2 pos = window->DC.CursorPos;
    const ImRect total_bb(pos, ImVec2(pos.x + square_sz + (label_size.x > 0 ? style.ItemInnerSpacing.x + label_size.x : 0), pos.y + square_sz + 5));

    ImGui::ItemSize(total_bb, style.FramePadding.y);
    if (!ImGui::ItemAdd(total_bb, id)) return;

    bool hovered, held;
    bool pressed = ImGui::ButtonBehavior(total_bb, id, &hovered, &held);
    if (pressed) *v = !(*v);

    ImU32 col = *v ? ImColor(255, 50, 50) : ImColor(40, 40, 40);
    window->DrawList->AddRectFilled(pos, ImVec2(pos.x + square_sz, pos.y + square_sz), col, 2.0f);

    if (*v) {
        window->DrawList->AddLine(ImVec2(pos.x + 4, pos.y + 8), ImVec2(pos.x + 7, pos.y + 11), ImColor(255, 255, 255), 2.0f);
        window->DrawList->AddLine(ImVec2(pos.x + 7, pos.y + 11), ImVec2(pos.x + 12, pos.y + 5), ImColor(255, 255, 255), 2.0f);
    }

    window->DrawList->AddText(ImVec2(pos.x + square_sz + 8, pos.y + 1), ImColor(200, 200, 200), label);
}

void CustomUI::Combo(const char* label, int* current_item, const char* const items[], int items_count)
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = ImGui::GetCurrentWindow();

    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.12f, 0.12f, 0.12f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(0.15f, 0.15f, 0.15f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImVec4(0.12f, 0.12f, 0.12f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.15f, 0.15f, 0.15f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(1.0f, 0.20f, 0.20f, 1.00f));

    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 2.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(4, 4));

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    g.Style.WindowBorderSize = 1.0f;

    ImGui::PushItemWidth(80);

    if (ImGui::BeginCombo(label, items[*current_item], ImGuiComboFlags_NoArrowButton | ImGuiComboFlags_HeightRegular))
    {
        for (int n = 0; n < items_count; n++)
        {
            bool is_selected = (*current_item == n);
            if (ImGui::Selectable(items[n], is_selected))
                *current_item = n;

            if (is_selected)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }

    ImVec2 min = ImGui::GetItemRectMin();
    ImVec2 max = ImGui::GetItemRectMax();

    ImGui::PushFont(NULL);
    window->DrawList->AddText(ImVec2(max.x - 14, min.y + 2), ImColor(255, 255, 255), "v");
    ImGui::PopFont();

    ImGui::PopItemWidth();
    ImGui::PopStyleVar(3);
    ImGui::PopStyleColor(5);
}

void CustomUI::ColorEdit(const char* label, float col[4])
{
    ImGui::PushID(label);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 2.0f);

    if (ImGui::ColorButton("##ColorBtn", *(ImVec4*)col, ImGuiColorEditFlags_AlphaPreview | ImGuiColorEditFlags_NoTooltip, ImVec2(16, 16)))
        ImGui::OpenPopup("Picker");

    if (ImGui::BeginPopup("Picker"))
    {
        ImGui::ColorPicker4("##Picker", col, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoInputs);
        ImGui::EndPopup();
    }

    ImGui::PopStyleVar();
    ImGui::PopID();
}

void CustomUI::SliderFloat(const char* label, float* v, float v_min, float v_max, const char* format, const char* zero_text)
{
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems) return;

    ImGuiContext& g = *GImGui;
    const ImGuiID id = window->GetID(label);
    const float width = ImGui::GetContentRegionAvail().x;
    const float bar_h = 4.0f;
    const float handle_r = 4.5f;
    const float item_h = 28.0f;
    const ImVec2 pos = window->DC.CursorPos;

    const ImRect total_bb(pos, ImVec2(pos.x + width, pos.y + item_h));
    ImGui::ItemSize(total_bb, g.Style.FramePadding.y);
    if (!ImGui::ItemAdd(total_bb, id)) return;

    const float bar_y = pos.y + 20.0f;
    const float bar_x0 = pos.x + handle_r;
    const float bar_x1 = pos.x + width - handle_r;
    const ImRect bar_bb(ImVec2(pos.x, bar_y), ImVec2(pos.x + width, bar_y + bar_h));
    const ImRect grab_bb(ImVec2(bar_x0, bar_y), ImVec2(bar_x1, bar_y + bar_h));

    bool hovered, held;
    ImGui::ButtonBehavior(bar_bb, id, &hovered, &held, ImGuiButtonFlags_AllowOverlap);

    if (held)
    {
        float t = ImClamp((g.IO.MousePos.x - bar_x0) / (bar_x1 - bar_x0), 0.0f, 1.0f);
        *v = v_min + t * (v_max - v_min);
    }

    char value_buf[64];
    if (zero_text && *v <= v_min)
        snprintf(value_buf, sizeof(value_buf), "%s", zero_text);
    else
        snprintf(value_buf, sizeof(value_buf), format, *v);

    window->DrawList->AddText(ImVec2(pos.x, pos.y + 1), ImColor(220, 220, 220), label);
    float val_w = ImGui::CalcTextSize(value_buf).x;
    window->DrawList->AddText(ImVec2(pos.x + width - val_w, pos.y + 1), ImColor(220, 220, 220), value_buf);

    window->DrawList->AddRectFilled(ImVec2(bar_x0, bar_y), ImVec2(bar_x1, bar_y + bar_h), ImColor(35, 35, 35), 0.0f);

    float t_val = (v_max > v_min) ? ImClamp((*v - v_min) / (v_max - v_min), 0.0f, 1.0f) : 0.0f;
    float fill_x = bar_x0 + t_val * (bar_x1 - bar_x0);

    if (fill_x > bar_x0)
        window->DrawList->AddRectFilled(ImVec2(bar_x0, bar_y), ImVec2(fill_x, bar_y + bar_h), ImColor(100, 100, 100), 0.0f);

    window->DrawList->AddCircleFilled(ImVec2(fill_x, bar_y + bar_h * 0.5f), handle_r, ImColor(220, 220, 220));
}

void CustomUI::MultiCombo(const char* label, bool* selections, const char* const items[], int items_count)
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = ImGui::GetCurrentWindow();

    char preview[128] = "";
    for (int i = 0; i < items_count; i++)
    {
        if (selections[i])
        {
            if (preview[0]) strncat(preview, ", ", sizeof(preview) - strlen(preview) - 1);
            strncat(preview, items[i], sizeof(preview) - strlen(preview) - 1);
        }
    }
    if (!preview[0]) snprintf(preview, sizeof(preview), "None");

    ImGui::PushID(label);

    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.12f, 0.12f, 0.12f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(0.15f, 0.15f, 0.15f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImVec4(0.12f, 0.12f, 0.12f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.15f, 0.15f, 0.15f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(1.0f, 0.20f, 0.20f, 1.00f));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 2.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(4, 4));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    g.Style.WindowBorderSize = 1.0f;

    float combo_w = 160.0f;
    ImGui::PushItemWidth(combo_w);

    if (ImGui::BeginCombo("##mc", preview, ImGuiComboFlags_NoArrowButton | ImGuiComboFlags_HeightRegular))
    {
        for (int i = 0; i < items_count; i++)
        {
            ImGui::PushID(i);
            ImGui::Selectable(items[i], selections[i], ImGuiSelectableFlags_DontClosePopups);
            if (ImGui::IsItemClicked()) selections[i] = !selections[i];
            ImGui::PopID();
        }
        ImGui::EndCombo();
    }

    ImVec2 rmin = ImGui::GetItemRectMin();
    ImVec2 rmax = ImGui::GetItemRectMax();
    window->DrawList->AddText(ImVec2(rmax.x - 14, rmin.y + 2), ImColor(255, 255, 255), "v");

    ImGui::PopItemWidth();
    ImGui::PopStyleVar(3);
    ImGui::PopStyleColor(5);
    ImGui::PopID();
}