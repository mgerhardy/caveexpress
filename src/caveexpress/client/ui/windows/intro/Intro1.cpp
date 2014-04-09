#include "Intro1.h"
#include "engine/client/ui/nodes/UINodeBackButton.h"
#include "caveexpress/client/ui/nodes/UINodeBackground.h"
#include "caveexpress/client/ui/nodes/UINodeIntroBackground.h"
#include "engine/client/ui/UI.h"

Intro1::Intro1 (IFrontend* frontend) :
		UIWindow("intro1", frontend, WINDOW_FLAG_MODAL)
{
	_onPop = CMD_START;

	UINode *background = new UINodeIntroBackground(frontend);
	add(background);

	if (!wantBackButton())
		return;

	add(new UINodeBackButton(frontend, background));
}
