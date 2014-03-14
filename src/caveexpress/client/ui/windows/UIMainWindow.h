#pragma once

#include "engine/client/ui/windows/UIWindow.h"

// forward decl
class CampaignManager;
class UINodePanel;
class UINodeSprite;
class UINodeMainButton;
class ServiceProvider;

class UIMainWindow: public UIWindow {
private:
	UINodeSprite *_player;
	UINodeSprite *_grandpa;
	UINodeSprite *_mammut;
	void flyPlayer ();
	void moveSprite (UINodeSprite* sprite, float speed) const;
public:
	UIMainWindow (IFrontend *frontend, ServiceProvider& serviceProvider);

	// UIWindow
	bool onPush () override;
	void update (uint32_t deltaTime) override;
};
