#pragma once

#include "engine/client/ui/windows/UIWindow.h"

// forward decl
class ServiceProvider;
class CampaignManager;

class UISettingsWindow: public UIWindow {
private:
	UINode* addSection (UINode* centerUnderNode, UINode* background, const std::string& title, const std::string& option1,
			UINodeListener* option1Listener, const std::string& option2, UINodeListener* option2Listener);
public:
	UISettingsWindow (IFrontend *frontend, ServiceProvider& serviceProvider, CampaignManager& campaignManager);
};
