#pragma once

namespace ksd
{
	void SliderInt(const char* label, int* v, int v_min, int v_max, const char* format = nullptr, int remove = 0) noexcept;
	void SliderFloat(const char* label, float* v, float v_min, float v_max, const char* format = nullptr, float scale = 1.f) noexcept;
}
