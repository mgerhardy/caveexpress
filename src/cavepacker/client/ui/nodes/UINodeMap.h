#pragma once

#include "ui/nodes/IUINodeMap.h"

class ServiceProvider;

namespace cavepacker {

class CavePackerClientMap;

class UINodeMap: public IUINodeMap {
private:
	ServiceProvider& _serviceProvider;
	void requestDeadlocks();
public:
	UINodeMap (IFrontend *frontend, ServiceProvider& serviceProvider, CampaignManager& campaignManager, int x, int y, int width, int height, CavePackerClientMap& map);
	virtual ~UINodeMap ();
};

}
