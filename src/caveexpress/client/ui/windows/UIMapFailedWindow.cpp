#include "UIMapFailedWindow.h"
#include "ui/UI.h"
#include "ui/nodes/UINodeBackButton.h"
#include "ui/nodes/UINodeMainButton.h"
#include "ui/layouts/UIHBoxLayout.h"
#include "caveexpress/client/ui/nodes/UINodeBackgroundScene.h"
#include "ui/windows/main/ReplayNodeListener.h"
#include <string>

namespace caveexpress {

UIMapFailedWindow::UIMapFailedWindow (IFrontend *frontend, CampaignManager& campaignManager) :
		UIWindow(UI_WINDOW_MAPFAILED, frontend), _campaignManager(campaignManager)
{
	setInactiveAfterPush();

	_background = new UINodeBackgroundScene(frontend);
	if (System.hasTouch() && !wantBackButton())
		_background->setOnActivate(CMD_UI_POP);
	add(_background);

	_replayCampaign = new UINodeMainButton(frontend, tr("Retry"));
	const float gapBack = std::max(0.01f, getScreenPadding());
	_replayCampaign->alignTo(_background, NODE_ALIGN_BOTTOM | NODE_ALIGN_RIGHT, gapBack);
	_replayCampaign->addListener(UINodeListenerPtr(new ReplayNodeListener(_campaignManager)));
	add(_replayCampaign);

	if (!wantBackButton()) {
		return;
	}

	add(new UINodeBackButton(frontend, _background));
}

void UIMapFailedWindow::updateReason (bool isMultiplayer, const MapFailedReason& reason, const ThemeType& theme)
{
	_replayCampaign->setVisible(!isMultiplayer);
	_background->updateReason(reason, theme);
}

}
