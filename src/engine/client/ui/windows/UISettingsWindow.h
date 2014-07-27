#pragma once

#include "engine/client/ui/windows/UIWindow.h"

// forward decl
class ServiceProvider;
class CampaignManager;

class UISettingsWindow: public UIWindow {
protected:
	UINode* _background;
	ServiceProvider& _serviceProvider;
	CampaignManager& _campaignManager;
	UINode* addSection (UINode* centerUnderNode, UINode* background, const std::string& title, const std::string& option1,
			UINodeListener* option1Listener, const std::string& option2, UINodeListener* option2Listener);
	void addSections();
	void init();
public:
	UISettingsWindow (IFrontend *frontend, ServiceProvider& serviceProvider, CampaignManager& campaignManager);
};
