#include "gui.hpp"
#include "pages/pages.hpp"
#include "pages/config/config.hpp"
#include "framework/shell/shell.hpp"
#include "framework/styling.hpp"

#include <algorithm>

namespace
{
	float menuAlpha = 0.f;
	bool wasOpen = false;
}

void gui::Render() noexcept
{
	if (ImGui::IsKeyPressed(ImGuiKey_Insert, false))
		open = !open;

	if (open && !wasOpen)
		ksd::ResetChildScrollStates();
	wasOpen = open;

	const float dt = ImGui::GetIO().DeltaTime;
	const float animSpeed = static_cast<float>(vars::menuAnimSpeed);
	const float fadeSpeed = (animSpeed <= 0.f) ? 1.f : dt * animSpeed * 0.1f;
	if (!open && menuAlpha > 0.f)
		menuAlpha = std::max(menuAlpha - fadeSpeed, 0.f);
	if (open && menuAlpha < 1.f)
		menuAlpha = std::min(menuAlpha + fadeSpeed, 1.f);

	style::menuAlpha = static_cast<int>(menuAlpha * 255.f);
	if (!open && menuAlpha <= 0.f)
		return;

	ksd::BeginMain();

	const char* tabIcons[] = { "A", "C", "D", "E", "F", "H" };
	int& currentTab = ksd::CurrentTab();
	for (int i = 0; i < 6; ++i)
		ksd::TabButton(tabIcons[i], &currentTab, i, 6);

	pages::RenderTab(currentTab);

	ksd::EndMain();
}
