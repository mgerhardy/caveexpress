#include "Intro1.h"
#include "engine/client/ui/nodes/UINodeBackButton.h"
#include "caveexpress/client/ui/nodes/UINodeBackground.h"
#include "caveexpress/shared/constants/Commands.h"
#include "engine/client/ui/UI.h"

Intro1::Intro1 (IFrontend* frontend) :
		UIWindow("intro1", frontend, WINDOW_FLAG_MODAL)
{
	_onPush = _onPop = CMD_MAP_PAUSE;

	UINodeBackground *background = new UINodeBackground(frontend, tr("Introduction"));
	add(background);

	if (!wantBackButton())
		return;

	add(new UINodeBackButton(frontend, background));
}
