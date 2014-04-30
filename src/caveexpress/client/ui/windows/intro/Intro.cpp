#include "Intro.h"
#include "engine/common/Commands.h"
#include "engine/common/String.h"
#include "engine/client/ui/nodes/UINodeBackButton.h"
#include "engine/client/ui/nodes/UINodeSprite.h"
#include "engine/client/ui/nodes/UINodeBar.h"
#include "caveexpress/client/ui/nodes/UINodeBackground.h"
#include "caveexpress/client/ui/nodes/UINodeIntroBackground.h"
#include "engine/client/ui/layouts/UIVBoxLayout.h"
#include "engine/client/ui/layouts/UIHBoxLayout.h"
#include "engine/client/ui/UI.h"

IntroTypeDescription::IntroTypeDescription(UINode* parent, IFrontend* frontend, const EntityType& type, const Animation& animation, const std::string& text) :
		UINode(frontend) {
	setLayout(new UIHBoxLayout(0.01f, false, NODE_ALIGN_MIDDLE));
	UINodeSprite* sprite = new UINodeSprite(frontend, type, animation);
	const float wp = parent->getWidth() / 5.0f;
	sprite->setAspectRatioSize(wp, wp);
	add(sprite);
	UINodeLabel* label = new UINodeLabel(frontend, text, getFont(HUGE_FONT));
	label->setColor(colorBlack);
	add(label);
}

IntroBarDescription::IntroBarDescription(IFrontend* frontend, const Color& barColor, const std::string& text) :
		UINode(frontend) {
	setLayout(new UIHBoxLayout(0.01f, false, NODE_ALIGN_MIDDLE));
	const float barHeight = 12.0f / _frontend->getHeight();
	const float barWidth = 102.0f / _frontend->getWidth();
	UINodeBar* timeBar = new UINodeBar(_frontend);
	timeBar->setMax(100);
	timeBar->setCurrent(100);
	timeBar->setBarColor(barColor);
	timeBar->setBorder(true);
	timeBar->setBorderColor(colorWhite);
	timeBar->setSize(barWidth, barHeight);
	add(timeBar);
	UINodeLabel* label = new UINodeLabel(frontend, text, getFont(HUGE_FONT));
	label->setColor(colorBlack);
	add(label);
}

IntroBarDescription::IntroBarDescription(IFrontend* frontend, const std::string& text) :
		UINode(frontend) {
	setLayout(new UIHBoxLayout(0.01f, false, NODE_ALIGN_MIDDLE));
	const float barHeight = 12.0f / _frontend->getHeight();
	const float barWidth = 102.0f / _frontend->getWidth();
	UINodeBar* timeBar = new UINodeBar(_frontend);
	timeBar->setMax(100);
	timeBar->setCurrent(100);
	timeBar->setBorder(true);
	timeBar->setBorderColor(colorWhite);
	timeBar->setSize(barWidth, barHeight);
	add(timeBar);
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
	setInactiveAfterPush(1000L);
}

bool Intro::onKeyPress (int32_t key, int16_t modifier)
{
	if (!isActiveAfterPush())
		return false;

	UI::get().delayedPop();
	return UIWindow::onKeyPress(key, modifier);
}

bool Intro::onFingerPress (int64_t finger, uint16_t x, uint16_t y)
{
	if (!isActiveAfterPush())
		return false;

	UI::get().delayedPop();
	return UIWindow::onFingerPress(finger, x, y);
}

void Intro::init ()
{
	addIntroNodes(_panel);

	add(_panel);

	// TODO: a back button to start the map isn't that good ;)
	//add(new UINodeBackButton(_frontend, _background));
}
