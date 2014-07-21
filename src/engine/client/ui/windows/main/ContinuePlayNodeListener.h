#pragma once

#include "engine/common/campaign/CampaignManager.h"
#include "engine/client/ui/UI.h"
#include "engine/client/ui/windows/UIWindow.h"
#include "engine/common/ServiceProvider.h"
#include "engine/common/network/INetwork.h"
#include "engine/common/Logger.h"

class ContinuePlayNodeListener: public UINodeListener {
private:
	CampaignManager& _campaignManager;
	ServiceProvider& _serviceProvider;
public:
	ContinuePlayNodeListener (CampaignManager &campaignManager, ServiceProvider& serviceProvider) :
			_campaignManager(campaignManager), _serviceProvider(serviceProvider)
	{
	}

	void onClick () override
	{
		info(LOG_CLIENT, "Continue in campaign");
		if (_serviceProvider.getNetwork().isMultiplayer()) {
			UI::get().pop();
			return;
		}
		if (_campaignManager.isNewlyCompleted()) {
			info(LOG_CLIENT, "Finished the campaign");
			UI::get().popMain();
			UI::get().push(UI_WINDOW_GAMEFINISHED);
		} else {
			info(LOG_CLIENT, "Continue play");
			UI::get().pop();
			_campaignManager.continuePlay();
		}
	}
};
