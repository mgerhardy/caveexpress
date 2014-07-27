#pragma once

#include "engine/client/ui/windows/IUIMapWindow.h"

class UIMapWindow: public IUIMapWindow {
protected:
	UINode* getFingerControl () override;
	void initHudNodes () override;
public:
	UIMapWindow (IFrontend *frontend, ServiceProvider& serviceProvider, CampaignManager& campaignManager, ClientMap& map);
};
