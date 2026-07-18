#include "pages.hpp"

namespace pages
{
	void RenderTab(int tab) noexcept
	{
		switch (tab)
		{
		case 0: RenderLegit(); break;
		case 1: RenderVisuals(); break;
		case 2: RenderMisc(); break;
		case 3: RenderSkins(); break;
		case 4: RenderPlayers(); break;
		case 5: RenderConfig(); break;
		default: break;
		}
	}
}
