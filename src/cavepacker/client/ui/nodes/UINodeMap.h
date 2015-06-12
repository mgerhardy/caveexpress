#pragma once

#include "ui/nodes/IUINodeMap.h"

namespace cavepacker {

class CavePackerClientMap;

class UINodeMap: public IUINodeMap {
public:
	UINodeMap (IFrontend *frontend, ServiceProvider& serviceProvider, CampaignManager& campaignManager, int x, int y, int width, int height, CavePackerClientMap& map);
	virtual ~UINodeMap ();
};

}
