#pragma once

#include "engine/common/campaign/CampaignManager.h"
#include "engine/client/ui/UI.h"
#include "engine/client/ui/windows/UIWindow.h"
#include "engine/common/ServiceProvider.h"
#include "engine/common/network/INetwork.h"

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
		if (_serviceProvider.getNetwork().isMultiplayer()) {
			UI::get().pop();
			return;
		}
		if (_campaignManager.isNewlyCompleted()) {
			UI::get().popMain();
			UI::get().push(UI_WINDOW_GAMEFINISHED);
		} else {
			UI::get().pop();
			_campaignManager.continuePlay();
		}
	}
};
