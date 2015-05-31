#pragma once

#include "client/ui/windows/UISettingsWindow.h"

// forward decl
class ServiceProvider;
class CampaignManager;

class UICaveExpressSettingsWindow: public UISettingsWindow {
protected:
	CampaignManager& _campaignManager;
	UINode* addSections() override;
public:
	UICaveExpressSettingsWindow (IFrontend *frontend, ServiceProvider& serviceProvider, CampaignManager& campaignManager);
};
