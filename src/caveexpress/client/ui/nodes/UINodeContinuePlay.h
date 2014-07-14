#pragma once

#include "engine/client/ui/nodes/UINodeButtonImage.h"
#include "caveexpress/client/ui/windows/main/ContinuePlayNodeListener.h"

class UINodeContinuePlay: public UINodeButtonImage {
public:
	UINodeContinuePlay (IFrontend *frontend, CampaignManager& campaignManager, ServiceProvider& serviceProvider) :
		UINodeButtonImage(frontend, "ui-button-start")
	{
		addListener(UINodeListenerPtr(new ContinuePlayNodeListener(campaignManager, serviceProvider)));
	}
};
