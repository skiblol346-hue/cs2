#include "players.hpp"
#include "../../framework/shell/shell.hpp"

namespace vars
{
	int playerPriority = 0;
}

namespace pages
{
	void RenderPlayers() noexcept
	{
		if (ksd::BeginChild("Players", { 0, 0, 3, 10 }))
			ksd::Label("Player list (demo)");
		ksd::EndChild();

		if (ksd::BeginChild("Adjustments", { 6, 0, 3, 10 }))
			ksd::SliderInt("Priority", &vars::playerPriority, 0, 10, "%d");
		ksd::EndChild();
	}
}
