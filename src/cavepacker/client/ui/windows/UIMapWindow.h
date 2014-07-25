#pragma once

#include "engine/client/ui/windows/IUIMapWindow.h"

class UIMapWindow: public IUIMapWindow {
protected:
	UINode* getFingerControl () override;
public:
	UIMapWindow (IFrontend *frontend, ServiceProvider& serviceProvider, CampaignManager& campaignManager, ClientMap& map);
};
