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

	UINode* child = new UINode(frontend);
	child->setBackgroundColor(colorGrayAlpha40);
	const float padding = 2 * 10.0f / std::max(_frontend->getWidth(), _frontend->getHeight());
	child->setSize(background->getWidth() - padding, background->getHeight() - padding);
	child->alignTo(background, NODE_ALIGN_CENTER | NODE_ALIGN_MIDDLE);
	add(child);

	addIntroNodes(child);

	if (!wantBackButton())
		return;

	add(new UINodeBackButton(frontend, child));
}
