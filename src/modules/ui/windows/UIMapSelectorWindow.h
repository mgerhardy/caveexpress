#pragma once

#include "ui/windows/UIWindow.h"
#include "client/sprites/Sprite.h"

// forward decl
class UINodeMapSelector;
class UINodeButton;
class UINodeSprite;

class UIMapSelectorWindow: public UIWindow {
protected:
	UINodeMapSelector* _mapSelector;
	UINodeButton *_buttonLeft;
	UINodeButton *_buttonRight;
	UINodeSprite *_livesSprite;
	SpritePtr _liveSprite;
public:
	UIMapSelectorWindow (UINodeMapSelector* mapSelector, const std::string& title, const std::string& id, IFrontend *frontend, WindowFlags flags =
			WINDOW_FLAG_FULLSCREEN);
	virtual ~UIMapSelectorWindow ();

	// UIWindow
	void update (uint32_t deltaTime) override;
	void onActive () override;
};
