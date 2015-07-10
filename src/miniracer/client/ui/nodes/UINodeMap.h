#pragma once

#include "ui/nodes/IUINodeMap.h"

namespace miniracer {

class MiniRacerClientMap;

class UINodeMap: public IUINodeMap {
public:
	UINodeMap (IFrontend *frontend, ServiceProvider& serviceProvider, CampaignManager& campaignManager, int x, int y, int width, int height, MiniRacerClientMap& map);
	virtual ~UINodeMap ();
};

}
