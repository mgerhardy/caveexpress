#include "UIGameOverWindow.h"
#include "engine/client/ui/UI.h"
#include "engine/client/ui/nodes/UINodeBackButton.h"
#include "engine/common/Logger.h"
#include "caveexpress/client/ui/nodes/UINodeGameOverBackground.h"
#include "engine/common/campaign/CampaignManager.h"
#include <string>

UIGameOverWindow::UIGameOverWindow (IFrontend *frontend, CampaignManager& campaignManager) :
		UIWindow(UI_WINDOW_GAMEOVER, frontend), _campaignManager(campaignManager)
{
	setInactiveAfterPush();

	UINode* background = new UINodeGameOverBackground(frontend);
	add(background);

	if (!wantBackButton())
		return;

	add(new UINodeBackButton(frontend, background));
}

UIGameOverWindow::~UIGameOverWindow ()
{
}

bool UIGameOverWindow::onPush ()
{
	const bool retVal = UIWindow::onPush();
	if (!_campaignManager.resetActiveCampaign())
		error(LOG_CLIENT, "failed to reset the campaign");
	return retVal;
}
