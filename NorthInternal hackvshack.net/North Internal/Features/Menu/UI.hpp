#pragma once
#include "../../Dependencies/ImGui/imgui.h"
#include "../../Dependencies/ImGui/imgui_internal.h"

namespace CustomUI {
    void Checkbox(const char* label, bool* v);
    void Combo(const char* label, int* current_item, const char* const items[], int items_count);
    void ColorEdit(const char* label, float col[4]);
    void SliderFloat(const char* label, float* v, float v_min, float v_max, const char* format = "%.2f", const char* zero_text = nullptr);
    void MultiCombo(const char* label, bool* selections, const char* const items[], int items_count);
}