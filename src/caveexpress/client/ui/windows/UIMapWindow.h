#pragma once

#include "ui/windows/IUIMapWindow.h"

namespace caveexpress {

class UIMapWindow: public IUIMapWindow {
public:
	UIMapWindow (IFrontend *frontend, ServiceProvider& serviceProvider, CampaignManager& campaignManager, ClientMap& map);
	virtual void initHudNodes() override;
};

}
