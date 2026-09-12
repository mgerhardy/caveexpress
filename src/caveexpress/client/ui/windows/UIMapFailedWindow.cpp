#include "UIMapFailedWindow.h"
#include "ui/UI.h"
#include "common/Commands.h"
#include "ui/nodes/UINodeBackButton.h"
#include "ui/nodes/UINodeMainButton.h"
#include "caveexpress/client/ui/nodes/UINodeBackgroundScene.h"
#include "ui/windows/main/ReplayNodeListener.h"
#include "common/ConfigManager.h"
#include <string>

namespace caveexpress {

UIMapFailedWindow::UIMapFailedWindow (IFrontend *frontend, CampaignManager& campaignManager) :
		UIWindow(UI_WINDOW_MAPFAILED, frontend, WINDOW_FLAG_FULLSCREEN | WINDOW_FLAG_MODAL), _campaignManager(campaignManager)
{
	setInactiveAfterPush();

	_background = new UINodeBackgroundScene(frontend);
	if (System.hasTouch() && !wantBackButton())
		_background->setOnActivate(CMD_UI_POP);
	add(_background);

	_replayCampaign = new UINodeMainButton(frontend, tr("Retry"));
	const float gapBack = std::max(0.01f, getScreenPadding());
	_replayCampaign->setId("retrybutton");
	_replayCampaign->alignTo(_background, NODE_ALIGN_BOTTOM | NODE_ALIGN_RIGHT, gapBack);
	_replayCampaign->addListener(UINodeListenerPtr(new ReplayNodeListener(_campaignManager)));
	add(_replayCampaign);

	_returnToLobby = new UINodeMainButton(frontend, tr("Continue"));
	_returnToLobby->setId("lobbybutton");
	_returnToLobby->alignTo(_background, NODE_ALIGN_BOTTOM | NODE_ALIGN_RIGHT, gapBack);
	_returnToLobby->setOnActivate(std::string(CMD_UI_POP) + ";" + CMD_RETURN_TO_LOBBY);
	_returnToLobby->setVisible(false);
	add(_returnToLobby);

	if (!wantBackButton()) {
		return;
	}

	add(new UINodeBackButton(frontend, _background));
}

void UIMapFailedWindow::onActive ()
{
	UIWindow::onActive();
	Config.setBindingsSpace(BINDINGS_UI);
}

void UIMapFailedWindow::updateReason (bool isMultiplayer, const MapFailedReason& reason, const ThemeType& theme)
{
	_replayCampaign->setVisible(!isMultiplayer);
	_returnToLobby->setVisible(isMultiplayer);
	_background->updateReason(reason, theme);
}

}
