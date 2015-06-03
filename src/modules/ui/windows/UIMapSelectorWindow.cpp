#include "UIMapSelectorWindow.h"
#include "ui/nodes/UINodeBackButton.h"
#include "ui/nodes/UINodeButton.h"
#include "ui/nodes/UINodeButtonImage.h"
#include "ui/nodes/UINodeSprite.h"
#include "ui/windows/listener/SelectorPageListener.h"
#include "ui/UI.h"
#include "common/SpriteDefinition.h"
#include "common/ConfigManager.h"
#include "ui/nodes/UINodeBackground.h"
#include "ui/nodes/UINodeMapSelector.h"
#include "campaign/persister/IGameStatePersister.h"

UIMapSelectorWindow::UIMapSelectorWindow (UINodeMapSelector* mapSelector, const std::string& title, const std::string& id, IFrontend *frontend,
		WindowFlags flags) :
		UIWindow(id, frontend, flags), _mapSelector(mapSelector)
{
	UINodeBackground *background = new UINodeBackground(frontend, title);
	add(background);

	const float gap = 0.001f;

	const int spriteHeight = 60;
	_livesSprite = new UINodeSprite(frontend, spriteHeight, spriteHeight);
	_livesSprite->setSpriteOffset(spriteHeight);
	_liveSprite = UI::get().loadSprite("icon-heart");
	for (uint8_t i = 0; i < INITIAL_LIVES; ++i) {
		_livesSprite->addSprite(_liveSprite);
	}
	_livesSprite->putAbove(_mapSelector);
	add(_livesSprite);

	_buttonLeft = new UINodeButtonImage(frontend, "icon-scroll-page-left");
	_buttonLeft->addListener(UINodeListenerPtr(new SelectorPageListener<std::string>(_mapSelector, false)));
	add(_mapSelector);

	_buttonLeft->setPos(_mapSelector->getLeft() - _buttonLeft->getWidth() - gap, _mapSelector->getTop());
	add(_buttonLeft);

	_buttonRight = new UINodeButtonImage(frontend, "icon-scroll-page-right");
	_buttonRight->setPos(_mapSelector->getRight() + gap, _mapSelector->getTop());
	_buttonRight->addListener(UINodeListenerPtr(new SelectorPageListener<std::string>(_mapSelector, true)));
	add(_buttonRight);

	_buttonRight->setVisible(false);
	_buttonLeft->setVisible(false);

	if (!wantBackButton())
		return;

	add(new UINodeBackButton(frontend, background));
}

UIMapSelectorWindow::~UIMapSelectorWindow ()
{
}

void UIMapSelectorWindow::update (uint32_t deltaTime)
{
	UIWindow::update(deltaTime);
	_buttonRight->setVisible(_mapSelector->hasMoreEntries());
	_buttonLeft->setVisible(_mapSelector->hasLessEntries());
}

void UIMapSelectorWindow::onActive ()
{
	UIWindow::onActive();
	_mapSelector->reset();

	if (Config.isModeEasy()) {
		_livesSprite->setVisible(false);
		return;
	}

	const int lives = _mapSelector->getLives();
	_livesSprite->setVisible(lives > 0);
	if (lives > 0) {
		_livesSprite->clearSprites();
		for (int i = 0; i < lives; ++i) {
			_livesSprite->addSprite(_liveSprite);
		}
	}
}
