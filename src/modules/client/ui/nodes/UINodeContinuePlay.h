#pragma once

#include "client/ui/nodes/UINodeButtonImage.h"
#include "client/ui/windows/main/ContinuePlayNodeListener.h"

class UINodeContinuePlay: public UINodeButtonImage {
public:
	UINodeContinuePlay (IFrontend *frontend, CampaignManager& campaignManager, ServiceProvider& serviceProvider) :
		UINodeButtonImage(frontend, "ui-button-start")
	{
		addListener(UINodeListenerPtr(new ContinuePlayNodeListener(campaignManager, serviceProvider)));
	}
};
