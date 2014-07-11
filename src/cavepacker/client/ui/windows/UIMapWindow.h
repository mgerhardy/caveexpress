#pragma once

#include "engine/client/ui/windows/UIWindow.h"

class UINode;
class UINodePoint;
class CampaignManager;
class ServiceProvider;
class ClientMap;
class UINodeMap;

class UIMapWindow: public UIWindow {
private:
	UINodeMap *_nodeMap;
	UINode *_panel;
	UINodePoint *_points;
	ServiceProvider& _serviceProvider;
public:
	UIMapWindow (IFrontend *frontend, ServiceProvider& serviceProvider, CampaignManager& campaignManager, ClientMap& map);
};
