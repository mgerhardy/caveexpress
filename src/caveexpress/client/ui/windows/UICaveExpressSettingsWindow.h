#pragma once

#include "ui/windows/UISettingsWindow.h"

// forward decl
class ServiceProvider;
class CampaignManager;
class UINodeButton;

namespace caveexpress {

class UICaveExpressSettingsWindow: public UISettingsWindow {
protected:
	CampaignManager& _campaignManager;
	UINodeButton* _gameModeNormal;
	UINodeButton* _gameModeHard;

	UINode* addSections() override;
public:
	UICaveExpressSettingsWindow (IFrontend *frontend, ServiceProvider& serviceProvider, CampaignManager& campaignManager);

	virtual void update(uint32_t time) override;
};

}
