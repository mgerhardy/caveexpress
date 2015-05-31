#pragma once

#include "ui/windows/UIWindow.h"

class CampaignManager;

class UIGameOverWindow: public UIWindow {
private:
	CampaignManager& _campaignManager;
public:
	UIGameOverWindow (IFrontend *frontend, CampaignManager& campaignManager);
	virtual ~UIGameOverWindow ();

	// UIWindows
	bool onPush () override;
};
