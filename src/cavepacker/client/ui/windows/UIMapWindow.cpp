#include "UIMapWindow.h"
#include "cavepacker/client/ui/nodes/UINodeMap.h"

UIMapWindow::UIMapWindow (IFrontend *frontend, ServiceProvider& serviceProvider, CampaignManager& campaignManager, ClientMap& map) :
	IUIMapWindow(frontend, serviceProvider, campaignManager, map, new UINodeMap(frontend, serviceProvider, campaignManager, 0, 0, frontend->getWidth(), frontend->getHeight(), map))
{
}
