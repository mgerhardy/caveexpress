#include "UINodeMap.h"

UINodeMap::UINodeMap (IFrontend *frontend, ServiceProvider& serviceProvider, CampaignManager& campaignManager, int x, int y, int width, int height, ClientMap& map) :
		IUINodeMap(frontend, serviceProvider, campaignManager, x, y, width, height, map)
{
}

UINodeMap::~UINodeMap ()
{
}
