#include "UIMapOptionsWindow.h"
#include "ui/nodes/UINodeBackButton.h"
#include "ui/nodes/UINodeBackToRootButton.h"
#include "ui/nodes/UINodeButtonText.h"
#include "service/ServiceProvider.h"
#include "common/Commands.h"
#include "network/INetwork.h"
#include "ui/nodes/UINodeSettingsBackground.h"
#include "ui/layouts/UIVBoxLayout.h"

UIMapOptionsWindow::UIMapOptionsWindow (IFrontend *frontend, ServiceProvider& serviceProvider) :
		UIWindow(UI_WINDOW_OPTIONS, frontend, WINDOW_FLAG_MODAL), _serviceProvider(serviceProvider)
{
	_onPush = _onPop = CMD_MAP_PAUSE;

	UINodeSettingsBackground *background = new UINodeSettingsBackground(frontend, "");
	background->setAmountHorizontal(1);
	add(background);

	_panel = new UINode(frontend, "panelMapSettings");
	_panel->setSize(background->getWidth(), background->getHeight());
	_panel->setLayout(new UIVBoxLayout(0.01f, true));
	_panel->alignTo(background, NODE_ALIGN_MIDDLE | NODE_ALIGN_CENTER);
	add(_panel);

	UINodeBackToRootButton* backToRoot = new UINodeBackToRootButton(frontend);
	const float gap = std::max(0.01f, getScreenPadding());
	backToRoot->alignTo(background, NODE_ALIGN_CENTER | NODE_ALIGN_TOP, gap);
	_panel->add(backToRoot);

	_restartMap = new UINodeMainButton(frontend, tr("Restart"));
	_restartMap->setId("restart-map");
	_restartMap->putUnder(backToRoot, 0.02f);
	_restartMap->setOnActivate(CMD_UI_POP ";" CMD_MAP_RESTART);
	_panel->add(_restartMap);

	if (!wantBackButton()) {
		_backButton = nullptr;
		return;
	}

	_backButton = new UINodeBackButton(frontend, background);
	_panel->add(_backButton);
}

void UIMapOptionsWindow::update (uint32_t deltaTime)
{
	UIWindow::update(deltaTime);
	const bool server = _serviceProvider.getNetwork().isServer();
	_restartMap->setVisible(server);
}

void UIMapOptionsWindow::onActive ()
{
	showAds();
	UIWindow::onActive();
	addLastFocus();
}

bool UIMapOptionsWindow::onPop ()
{
	hideAds();
	return UIWindow::onPop();
}
