#include "../pages.hpp"
#include "../../gui.hpp"

namespace vars
{
	inline float colorAccent[4] = { 0.64f, 0.83f, 0.12f, 1.0f };
}

namespace pages
{
	void RenderMain() noexcept
	{
		ImGui::TextUnformatted("Main page");
		ImGui::Spacing();
		ImGui::ColorEdit4("Accent color", vars::colorAccent);

		static bool isSampleEnabled = true;
		ImGui::Checkbox("Sample option", &isSampleEnabled);
	}
}
