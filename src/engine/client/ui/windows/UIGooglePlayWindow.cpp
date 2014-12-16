#include "UIGooglePlayWindow.h"
#include "engine/client/ui/UI.h"
#include "engine/client/ui/nodes/UINodeBackButton.h"
#include "engine/client/ui/nodes/UINodeLabel.h"
#include "engine/client/ui/nodes/UINodeBackground.h"
#include "engine/client/ui/nodes/UINodeMainButton.h"
#include "engine/client/ui/nodes/UINodeGooglePlayButton.h"
#include "engine/client/ui/layouts/UIVBoxLayout.h"
#include "engine/client/ui/nodes/UINodeButtonImage.h"
#include "engine/client/ui/windows/listener/GooglePlayDisconnectListener.h"

UIGooglePlayWindow::UIGooglePlayWindow (IFrontend *frontend) :
		UIWindow(UI_WINDOW_GOOGLEPLAY, frontend, WINDOW_FLAG_MODAL)
{
	_googlePlay = Config.getConfigVar("googleplaystate");
	UINode* background = new UINodeBackground(frontend, tr("Google Play"));
	add(background);

	UINode* panel = new UINode(frontend);
	panel->setAlignment(NODE_ALIGN_MIDDLE | NODE_ALIGN_CENTER);
	UIVBoxLayout *layout = new UIVBoxLayout(0.02f, true);

	_achievements = new UINodeMainButton(frontend, tr("Achievements"));
	_achievements->setOnActivate("googleplay-showachievements");
	panel->add(_achievements);

#if 0
	_leaderBoards = new UINodeMainButton(frontend, tr("Leaderboard"));
	_leaderBoards->setOnActivate("googleplay-showleaderboard main");
	panel->add(_leaderBoards);
#endif

	_disconnect = new UINodeMainButton(frontend, tr("Disconnect"));
	_disconnect->addListener(UINodeListenerPtr(new GooglePlayDisconnectListener()));
	panel->add(_disconnect);

	panel->setLayout(layout);
	add(panel);

	const float padding = 10.0f / static_cast<float>(frontend->getHeight());
	_login = new UINodeButtonImage(frontend, "icon-google-login");
	_login->setAlignment(NODE_ALIGN_BOTTOM | NODE_ALIGN_LEFT);
	_login->setOnActivate("googleplay-connect");
	_login->setPadding(padding);
	add(_login);

	if (!wantBackButton())
		return;

	add(new UINodeBackButton(frontend, background));
}

void UIGooglePlayWindow::update (uint32_t deltaTime)
{
	UIWindow::update(deltaTime);
	const bool state = _googlePlay->getBoolValue();
	_achievements->setVisible(state);
#if 0
	_leaderBoards->setVisible(state);
#endif
	_disconnect->setVisible(state);
	_login->setVisible(!state);
}

bool UIGooglePlayWindow::onPush () {
	if (!UIWindow::onPush()) {
		return false;
	}
	return System.supportGooglePlay();
}
