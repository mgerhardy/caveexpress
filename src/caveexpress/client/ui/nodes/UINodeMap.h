#pragma once

#include "engine/client/ui/nodes/IUINodeMap.h"

class UINodeMap: public IUINodeMap {
public:
	UINodeMap (IFrontend *frontend, ServiceProvider& serviceProvider, CampaignManager& campaignManager, int x, int y, int width, int height, ClientMap& map);
	virtual ~UINodeMap ();
};
