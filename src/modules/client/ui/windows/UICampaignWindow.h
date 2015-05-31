#pragma once

#include "client/ui/windows/UIWindow.h"

// forward decl
class CampaignManager;
class ServiceProvider;
class UINodeButton;
class UINodeCampaignSelector;

class UICampaignWindow: public UIWindow {
private:
	CampaignManager& _campaignManager;
	UINode *_continuePlay;
	UINodeButton *_buttonLeft;
	UINodeButton *_buttonRight;
	UINodeCampaignSelector* _campaign;
public:
	UICampaignWindow (IFrontend *frontend, ServiceProvider& serviceProvider, CampaignManager& campaignManager, int cols = 6, int rows = 4);
	virtual ~UICampaignWindow ();

	// UIWindow
	void onActive () override;
	void update (uint32_t deltaTime) override;
};
