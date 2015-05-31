#pragma once

#include "client/ui/windows/UIWindow.h"
#include "caveexpress/shared/CaveExpressAnimation.h"
#include "common/EntityType.h"

class UINodeSprite;

class UIGameHelpWindow: public UIWindow {
private:
	UINodeSprite* createSprite (const EntityType& type, const Animation& animation = Animations::ANIMATION_IDLE, float w = 0.08f, float h = 0.08f);
	UINode* createTexture (const std::string& texture);
	UINode* createHPanel ();

	void addStoneWalkingHelp (UINode *panel);
	void addStoneFlyingHelp (UINode *panel);
	void addPackageHelp (UINode *panel);
	void addTreeHelp (UINode *panel);
	void addLivesHelp (UINode *panel);
	void addOuyaButton (UINode *panel, const std::string& texture, const std::string& title);

public:
	UIGameHelpWindow (IFrontend* frontend);
	virtual ~UIGameHelpWindow ();
};
