#include "UIGameOverWindow.h"
#include "ui/UI.h"
#include "ui/nodes/UINodeBackButton.h"
#include "common/Log.h"
#include "caveexpress/client/ui/nodes/UINodeGameOverBackground.h"
#include "campaign/CampaignManager.h"
#include <string>

namespace caveexpress {

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
		Log::error(LOG_CLIENT, "failed to reset the campaign");
	return retVal;
}

}
