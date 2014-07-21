#include "UIMapOptionsWindow.h"
#include "engine/client/ui/nodes/UINodeBackButton.h"
#include "engine/client/ui/nodes/UINodeBackToRootButton.h"
#include "engine/client/ui/nodes/UINodeButtonText.h"
#include "engine/common/ServiceProvider.h"
#include "engine/common/Commands.h"
#include "engine/common/network/INetwork.h"
#include "engine/client/ui/nodes/UINodeSettingsBackground.h"

UIMapOptionsWindow::UIMapOptionsWindow (IFrontend *frontend, ServiceProvider& serviceProvider) :
		UIWindow(UI_WINDOW_OPTIONS, frontend, WINDOW_FLAG_MODAL), _serviceProvider(serviceProvider)
{
	_onPush = _onPop = CMD_MAP_PAUSE;

	UINodeSettingsBackground *background = new UINodeSettingsBackground(frontend, "");
	background->setAmountHorizontal(1);
	add(background);

	UINodeBackToRootButton* backToRoot = new UINodeBackToRootButton(frontend);
	const float gap = std::max(0.01f, getScreenPadding());
	backToRoot->alignTo(background, NODE_ALIGN_CENTER | NODE_ALIGN_TOP, gap);
	add(backToRoot);

	_restartMap = new UINodeButtonImage(frontend, "icon-reload");
	_restartMap->setId("restart-map");
	_restartMap->putUnder(backToRoot, 0.02f);
	_restartMap->setOnActivate(CMD_UI_POP ";" CMD_MAP_RESTART);
	add(_restartMap);

	if (!wantBackButton())
		return;

	UINodeBackButton *backButton = new UINodeBackButton(frontend);
	const float gapBack = std::max(0.01f, getScreenPadding());
	backButton->alignTo(background, NODE_ALIGN_BOTTOM | NODE_ALIGN_LEFT, gapBack);
	add(backButton);
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
}

bool UIMapOptionsWindow::onPop ()
{
	hideAds();
	return UIWindow::onPop();
}
