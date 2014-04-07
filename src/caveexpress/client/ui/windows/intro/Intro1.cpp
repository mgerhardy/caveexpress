#include "Intro1.h"
#include "engine/client/ui/nodes/UINodeBackButton.h"
#include "caveexpress/client/ui/nodes/UINodeBackground.h"
#include "caveexpress/shared/constants/Commands.h"

Intro1::Intro1 (IFrontend* frontend) :
		UIWindow("intro1", frontend, WINDOW_FLAG_FULLSCREEN)
{
	_onPush = _onPop = CMD_MAP_PAUSE;

	UINodeBackground *background = new UINodeBackground(frontend, tr("Introduction"));
	add(background);

	if (!wantBackButton())
		return;

	add(new UINodeBackButton(frontend, background));
}

Intro1::~Intro1 ()
{
}

bool Intro1::onPop () {
	return UIWindow::onPop();
}

void Intro1::onActive ()
{
	UIWindow::onActive();
}
