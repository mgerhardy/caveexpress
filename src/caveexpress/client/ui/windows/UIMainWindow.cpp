#include "UIMainWindow.h"
#include "engine/client/ui/UI.h"
#include "engine/client/ui/nodes/UINodeButton.h"
#include "engine/client/ui/nodes/UINodeMainButton.h"
#include "caveexpress/client/ui/nodes/UINodeMainBackground.h"
#include "engine/client/ui/nodes/UINodeSprite.h"
#include "engine/client/ui/nodes/UINodeLabel.h"
#include "engine/client/ui/windows/listener/QuitListener.h"
#include "engine/client/ui/layouts/UIVBoxLayout.h"
#include "engine/common/ConfigManager.h"
#include "engine/common/Version.h"
#include "engine/common/System.h"
#include "engine/client/ui/windows/listener/OpenWindowListener.h"

UIMainWindow::UIMainWindow (IFrontend *frontend, ServiceProvider& serviceProvider) :
		UIWindow(UI_WINDOW_MAIN, frontend, WINDOW_FLAG_ROOT)
{
	add(new UINodeMainBackground(frontend, false));
	const SpritePtr& mammutSprite = UI::get().loadSprite("ui-npc-mammut");
	_mammut = new UINodeSprite(frontend, mammutSprite->getMaxWidth(), mammutSprite->getMaxHeight());
	_mammut->addSprite(mammutSprite);

	const SpritePtr& grandPaSprite = UI::get().loadSprite("ui-npc-grandpa");
	_grandpa = new UINodeSprite(frontend, grandPaSprite->getMaxWidth(), grandPaSprite->getMaxHeight());
	_grandpa->addSprite(grandPaSprite);

	const SpritePtr& playerSprite = UI::get().loadSprite("ui-player");
	_player = new UINodeSprite(frontend, playerSprite->getMaxWidth(), playerSprite->getMaxHeight());
	_player->addSprite(playerSprite);

	add(_mammut);
	add(_grandpa);
	add(_player);

	const float padding = 10.0f / static_cast<float>(_frontend->getHeight());
	UINode *panel = new UINode(_frontend, "panelMain");
	UIVBoxLayout *layout = new UIVBoxLayout(padding, true);
	panel->setLayout(layout);
	panel->setAlignment(NODE_ALIGN_MIDDLE | NODE_ALIGN_CENTER);
	panel->setPadding(padding);

	UINodeMainButton *campaign = new UINodeMainButton(_frontend, tr("Campaign"));
	campaign->addListener(UINodeListenerPtr(new OpenWindowListener(UI_WINDOW_CAMPAIGN)));
	panel->add(campaign);

	if (Config.isNetwork()) {
		UINodeMainButton *multiplayer = new UINodeMainButton(_frontend, tr("Multiplayer"));
		multiplayer->addListener(UINodeListenerPtr(new OpenWindowListener(UI_WINDOW_MULTIPLAYER)));
		panel->add(multiplayer);
	}

	UINodeMainButton *settings = new UINodeMainButton(_frontend, tr("Settings"));
	settings->addListener(UINodeListenerPtr(new OpenWindowListener(UI_WINDOW_SETTINGS)));
	panel->add(settings);

	if (System.supportPayment()) {
		UINodeMainButton *payment = new UINodeMainButton(_frontend, tr("Extras"));
		payment->addListener(UINodeListenerPtr(new OpenWindowListener(UI_WINDOW_PAYMENT)));
		panel->add(payment);
	}

	UINodeMainButton *twitter = new UINodeMainButton(_frontend, tr("Twitter"));
	twitter->addListener(UINodeListenerPtr(new OpenURLListener(_frontend, "https://twitter.com/MartinGerhardy")));
	panel->add(twitter);

	UINodeMainButton *facebook = new UINodeMainButton(_frontend, tr("Facebook"));
	facebook->addListener(UINodeListenerPtr(new OpenURLListener(_frontend, "https://facebook.com/" APPNAME)));
	panel->add(facebook);

	UINodeMainButton *help = new UINodeMainButton(_frontend, tr("Help"));
	help->addListener(UINodeListenerPtr(new OpenWindowListener(UI_WINDOW_HELP)));
	panel->add(help);

	UINodeMainButton *quit = new UINodeMainButton(_frontend, tr("Quit"));
	quit->addListener(UINodeListenerPtr(new QuitListener()));
	panel->add(quit);

	add(panel);

	UINodeLabel *versionLabel = new UINodeLabel(_frontend, VERSION);
	versionLabel->setAlignment(NODE_ALIGN_BOTTOM|NODE_ALIGN_RIGHT);
	versionLabel->setColor(colorWhite);
	versionLabel->setPadding(getScreenPadding());
	add(versionLabel);
}

bool UIMainWindow::onPush ()
{
	const bool ret = UIWindow::onPush();
	showAds();
	return ret;
}

void UIMainWindow::update (uint32_t deltaTime)
{
	UIWindow::update(deltaTime);
	if (!_player->isMovementActive()) {
		flyPlayer();
	}
	if (!_grandpa->isMovementActive()) {
		moveSprite(_grandpa, 0.00008f);
	}
	if (!_mammut->isMovementActive()) {
		moveSprite(_mammut, 0.0001f);
	}
}

void UIMainWindow::moveSprite (UINodeSprite* sprite, float speed) const
{
	const float y = 1.0f - (sprite->getRenderHeight() / static_cast<float>(_frontend->getHeight()));
	const float startX = randBetweenf(-2.2f, -0.2f);
	const float endX = randBetweenf(2.1f, 3.3f);
	sprite->setMovement(startX, y, endX, y, speed);
}

void UIMainWindow::flyPlayer ()
{
	const float spriteSize = _player->getRenderHeight() / static_cast<float>(_frontend->getHeight());
	const float y = 1.0f - spriteSize;
	const float startYRand = randBetweenf(0.0f, y);
	const float endYRand = randBetweenf(0.0f, y);
	const bool leftToRight = rand() % 2 == 0;
	float startX = randBetweenf(-4.5f, -0.5f);
	float endX = randBetweenf(2.5f, 4.5f);
	if (!leftToRight) {
		std::swap(startX, endX);
	}
	_player->setMovement(startX, startYRand, endX, endYRand, 0.0004f);
}
