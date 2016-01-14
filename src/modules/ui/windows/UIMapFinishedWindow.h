#pragma once

#include "ui/windows/UIWindow.h"
#include "common/SoundType.h"

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
	UIMapFinishedWindow (IFrontend *frontend, CampaignManager& campaignManager, ServiceProvider& serviceProvider, const SoundType& soundType);
	bool onPush () override;
	void onActive () override;
};
