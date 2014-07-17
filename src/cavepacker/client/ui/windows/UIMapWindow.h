#pragma once

#include "engine/client/ui/windows/IUIMapWindow.h"

class UIMapWindow: public IUIMapWindow {
public:
	UIMapWindow (IFrontend *frontend, ServiceProvider& serviceProvider, CampaignManager& campaignManager, ClientMap& map);
};
