#pragma once

#include "ui/nodes/IUINodeMap.h"

namespace caveexpress {

class UINodeMap: public IUINodeMap {
public:
	UINodeMap (IFrontend *frontend, ServiceProvider& serviceProvider, CampaignManager& campaignManager, int x, int y, int width, int height, ClientMap& map);
	virtual ~UINodeMap ();
};

}
