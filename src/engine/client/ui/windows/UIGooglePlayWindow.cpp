#include "UIGooglePlayWindow.h"
#include "engine/client/ui/UI.h"
#include "engine/client/ui/nodes/UINodeBackButton.h"
#include "engine/client/ui/nodes/UINodeLabel.h"
#include "engine/client/ui/nodes/UINodeBackground.h"
#include "engine/client/ui/layouts/UIVBoxLayout.h"

UIGooglePlayWindow::UIGooglePlayWindow (IFrontend *frontend) :
		UIWindow(UI_WINDOW_GOOGLEPLAY, frontend, WINDOW_FLAG_MODAL)
{
	_googlePlay = Config.getConfigVar("googleplaystate");
	UINode* background = new UINodeBackground(frontend, tr("Google Play"));
	add(background);

	UINode* panel = new UINode(frontend);
	UIVBoxLayout *layout = new UIVBoxLayout(0.02f);

	_achievements = new UINodeButtonText(frontend, tr("Show achievements"));
	_achievements->setOnActivate("google-showachievements");
	panel->add(_achievements);

#if 0
	_leaderBoards = new UINodeButtonText(frontend, tr("Show main leaderboard"));
	_leaderBoards->setOnActivate("google-showleaderboard main");
	panel->add(_leaderBoards);
#endif

	_disconnect = new UINodeButtonText(frontend, tr("Disconnect"));
	_disconnect->setOnActivate("google-disconnect");
	panel->add(_disconnect);

	panel->setLayout(layout);
	add(panel);

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
}
