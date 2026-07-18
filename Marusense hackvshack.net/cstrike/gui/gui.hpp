#pragma once

#include "imgui.h"

#define WIDTH 660
#define HEIGHT 560

namespace vars
{
	inline float colorAccent[4] = { 0.656f, 0.546f, 0.981f, 1.0f };
}

namespace gui
{
	inline bool open = false;
	void Render() noexcept;
}
