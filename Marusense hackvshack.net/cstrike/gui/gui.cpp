#include "gui.hpp"
#include "pages/pages.hpp"
#include "framework/shell/shell.hpp"
#include "framework/styling.hpp"
#include <Windows.h>
#include "core/variables.h"

#include <algorithm>

namespace
{
	float menuAlpha = 0.f;
	bool wasOpen = false;
}

void gui::Render() noexcept
{
	if (open && !wasOpen)
		ksd::ResetChildScrollStates();
	wasOpen = open;

	const float dt = ImGui::GetIO().DeltaTime;
	const float animSpeed = C_GET(float, Vars.flAnimationSpeed);
	const float fadeSpeed = (animSpeed <= 0.f) ? 1.f : dt * animSpeed;
	if (!open && menuAlpha > 0.f)
		menuAlpha = std::max(menuAlpha - fadeSpeed, 0.f);
	if (open && menuAlpha < 1.f)
		menuAlpha = std::min(menuAlpha + fadeSpeed, 1.f);

	style::menuAlpha = static_cast<int>(menuAlpha * 255.f);
	if (!open && menuAlpha <= 0.f)
		return;

	ksd::BeginMain();

	const char* tabIcons[] = { "A", "C", "D", "H" };
	int& currentTab = ksd::CurrentTab();
	for (int i = 0; i < 4; ++i)
		ksd::TabButton(tabIcons[i], &currentTab, i, 4);

	pages::RenderTab(currentTab);

	ksd::EndMain();
}
