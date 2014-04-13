#include "Intro.h"
#include "engine/common/Commands.h"
#include "engine/client/ui/nodes/UINodeBackButton.h"
#include "engine/client/ui/nodes/UINodeSprite.h"
#include "caveexpress/client/ui/nodes/UINodeBackground.h"
#include "caveexpress/client/ui/nodes/UINodeIntroBackground.h"
#include "engine/client/ui/layouts/UIVBoxLayout.h"
#include "engine/client/ui/layouts/UIHBoxLayout.h"
#include "engine/client/ui/UI.h"

IntroTypeDescription::IntroTypeDescription(IFrontend* frontend, const EntityType& type, const Animation& animation, const std::string& text) :
		UINode(frontend) {
	setLayout(new UIHBoxLayout(0.01f, false, NODE_ALIGN_MIDDLE));
	UINodeSprite* sprite = new UINodeSprite(frontend, type, animation);
	sprite->setAspectRatioSize(0.1f, 0.1f, 1.0f);
	add(sprite);
	UINodeLabel* label = new UINodeLabel(frontend, text, getFont(HUGE_FONT));
	label->setColor(colorBlack);
	add(label);
}

Intro::Intro(const std::string& name, IFrontend* frontend) :
		UIWindow(name, frontend, WINDOW_FLAG_MODAL) {
	_onPop = CMD_START;

	_background = new UINodeIntroBackground(frontend);
	UINode *overlay = new UINode(frontend);
	overlay->setBackgroundColor(colorWhiteAlpha80);
	const float padding = 4.0f / std::max(_frontend->getWidth(), _frontend->getHeight());
	overlay->setSize(_background->getWidth() - 2.0f * padding, _background->getHeight() - 2.0f * padding);
	overlay->setPos(padding, padding);
	_background->add(overlay);
	add(_background);

	_panel = new UINode(frontend);
	_panel->setStandardPadding();
	_panel->setPos(0.0f, _background->getTop());
	_panel->setSize(_background->getWidth(), _background->getHeight());
	_panel->setAlignment(NODE_ALIGN_CENTER | NODE_ALIGN_TOP);
	UIVBoxLayout *layout = new UIVBoxLayout(0.01f, true, NODE_ALIGN_CENTER);
	_panel->setLayout(layout);
}

void Intro::init ()
{
	addIntroNodes(_panel);

	add(_panel);

	// TODO: a back button to start the map isn't that good ;)
	if (!wantBackButton())
		return;

	add(new UINodeBackButton(_frontend, _background));
}
