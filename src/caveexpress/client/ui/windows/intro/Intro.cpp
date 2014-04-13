#include "Intro.h"
#include "engine/common/Commands.h"
#include "engine/client/ui/nodes/UINodeBackButton.h"
#include "caveexpress/client/ui/nodes/UINodeBackground.h"
#include "caveexpress/client/ui/nodes/UINodeIntroBackground.h"
#include "engine/client/ui/UI.h"

Intro::Intro(const std::string& name, IFrontend* frontend) :
		UIWindow(name, frontend, WINDOW_FLAG_MODAL) {
	_onPop = CMD_START;

	_background = new UINodeIntroBackground(frontend);
	add(_background);

	_child = new UINode(frontend);
	_child->setBackgroundColor(colorWhiteAlpha80);
	const float padding = 2 * 10.0f / std::max(_frontend->getWidth(), _frontend->getHeight());
	_child->setSize(_background->getWidth() - padding, _background->getHeight() - padding);
	_child->alignTo(_background, NODE_ALIGN_CENTER | NODE_ALIGN_MIDDLE);
	add(_child);

	if (!wantBackButton())
		return;

	add(new UINodeBackButton(frontend, _child));
}

void Intro::init ()
{
	addIntroNodes(_child);
}
