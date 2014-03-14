#pragma once

#include "engine/client/ui/windows/UIWindow.h"

#define UINODE_FINISHEDPOINTS "points"

// forward decl
class CampaignManager;
class ServiceProvider;
class UINodeButton;

class UIMapFinishedWindow: public UIWindow {
private:
	UINodeButton *_replayCampaign;
	ServiceProvider& _serviceProvider;
public:
	UIMapFinishedWindow (IFrontend *frontend, CampaignManager& campaignManager, ServiceProvider& serviceProvider);
	bool onPush () override;
};
