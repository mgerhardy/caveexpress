#include "UIViewResultWindow.h"
#include "engine/client/ui/UI.h"
#include "geophoto/client/ui/nodes/UINodeLocationSelector.h"

UIViewResultWindow::UIViewResultWindow (IFrontend *frontend, const GameRound& gameRound) :
		UIWindow(UI_WINDOW_VIEWRESULT, frontend, WINDOW_FLAG_MODAL), _gameRound(gameRound)
{
	UINode* background = new UINode(frontend);
	background->setImage("bg");
	background->setSize(1.0f, 1.0f);
	background->setPos(0.0f, 0.0f);
	add(background);

	_selector = new UINodeLocationSelector(frontend, gameRound);
	_selector->setSize(0.5f, 1.0f);
	add(_selector);
}

UIViewResultWindow::~UIViewResultWindow ()
{
}

bool UIViewResultWindow::onPush ()
{
	const bool val = UIWindow::onPush();
	_selector->reset();
	return val;
}
