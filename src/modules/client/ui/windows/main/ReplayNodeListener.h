#pragma once

#include "common/campaign/CampaignManager.h"
#include "client/ui/UI.h"
#include "client/ui/windows/UIWindow.h"

class ReplayNodeListener: public UINodeListener {
private:
	CampaignManager &_campaignManager;
public:
	ReplayNodeListener (CampaignManager &campaignManager) :
			_campaignManager(campaignManager)
	{
	}

	void onClick () override
	{
		UI::get().popMain();
		if (!_campaignManager.replay())
			UI::get().push(UI_WINDOW_CAMPAIGN);
	}
};
