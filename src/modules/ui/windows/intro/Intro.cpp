#include "Intro.h"
#include "common/Commands.h"
#include "common/ConfigManager.h"
#include "common/String.h"
#include "ui/nodes/UINodeBackButton.h"
#include "ui/nodes/UINodeSprite.h"
#include "ui/nodes/UINodeBar.h"
#include "ui/nodes/UINodeBackground.h"
#include "ui/nodes/UINodeIntroBackground.h"
#include "ui/layouts/UIVBoxLayout.h"
#include "ui/layouts/UIHBoxLayout.h"
#include "ui/UI.h"

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

	UINodeButton* close = new UINodeButton(_frontend);
	close->setImage("icon-close");
	close->setOnActivate(CMD_UI_POP);
	close->alignTo(_background, NODE_ALIGN_RIGHT | NODE_ALIGN_TOP, 0.01f);
	add(close);

	_panel = new UINode(frontend);
	_panel->setStandardPadding();
	_panel->setPos(0.0f, _background->getTop());
	_panel->setSize(_background->getWidth(), _background->getHeight());
	_panel->setAlignment(NODE_ALIGN_CENTER | NODE_ALIGN_TOP);
	UIVBoxLayout *layout = new UIVBoxLayout(0.01f, true, NODE_ALIGN_CENTER);
	_panel->setLayout(layout);
	setInactiveAfterPush(1000L);

	if (wantBackButton()) {
		UINodeMainButton *continueButton = new UINodeMainButton(frontend, tr("Continue"), LARGE_FONT, colorBlack);
		const float gapBack = std::max(0.01f, getScreenPadding());
		continueButton->alignTo(_background, NODE_ALIGN_BOTTOM | NODE_ALIGN_RIGHT, gapBack);
		continueButton->setOnActivate(CMD_UI_POP);
		add(continueButton);
	}
}

bool Intro::onPop ()
{
	const bool retVal = UIWindow::onPop();
	Config.setBindingsSpace(BINDINGS_MAP);
	return retVal;
}

void Intro::onActive ()
{
	UIWindow::onActive();
	addLastFocus();
	Config.setBindingsSpace(BINDINGS_UI);
}

bool Intro::onKeyPress (int32_t key, int16_t modifier)
{
	if (!isActiveAfterPush())
		return false;

	if (!wantBackButton())
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
}
