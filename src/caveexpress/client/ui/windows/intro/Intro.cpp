#include "Intro.h"
#include "engine/common/Commands.h"
#include "engine/client/ui/nodes/UINodeBackButton.h"
#include "caveexpress/client/ui/nodes/UINodeBackground.h"
#include "caveexpress/client/ui/nodes/UINodeIntroBackground.h"
#include "engine/client/ui/UI.h"

Intro::Intro(const std::string& name, IFrontend* frontend) :
		UIWindow(name, frontend, WINDOW_FLAG_MODAL) {
	_onPop = CMD_START;

	UINode *background = new UINodeIntroBackground(frontend);
	add(background);

	addIntroNodes();

	if (!wantBackButton())
		return;

	add(new UINodeBackButton(frontend, background));
}
