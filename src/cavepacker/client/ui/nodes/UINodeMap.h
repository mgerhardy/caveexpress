#pragma once

#include "client/ui/nodes/IUINodeMap.h"

class CavePackerClientMap;

class UINodeMap: public IUINodeMap {
public:
	UINodeMap (IFrontend *frontend, ServiceProvider& serviceProvider, CampaignManager& campaignManager, int x, int y, int width, int height, CavePackerClientMap& map);
	virtual ~UINodeMap ();
};
