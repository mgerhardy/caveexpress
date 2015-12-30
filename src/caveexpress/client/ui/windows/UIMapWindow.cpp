#include "UIMapWindow.h"
#include "caveexpress/client/ui/nodes/UINodeMap.h"

namespace caveexpress {

UIMapWindow::UIMapWindow (IFrontend *frontend, ServiceProvider& serviceProvider, CampaignManager& campaignManager, ClientMap& map) :
		IUIMapWindow(frontend, serviceProvider, campaignManager, new UINodeMap(frontend, serviceProvider, campaignManager, 0, 0, frontend->getWidth(), frontend->getHeight(), map))
{
	init();
}

}
