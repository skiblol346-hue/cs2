#pragma once

#include "imgui.h"

struct ID3D11Device;

namespace svg
{
	void Init(ID3D11Device* device) noexcept;

	ImTextureID Get(const char* name, int heightPx, int& outW, int& outH) noexcept;
}
