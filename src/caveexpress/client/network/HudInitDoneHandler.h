#pragma once

#include "caveexpress/shared/CaveExpressEntityType.h"
#include "network/IProtocolHandler.h"
#include "client/network/InitDoneHandler.h"
#include "caveexpress/shared/CaveExpressAnimation.h"
#include "common/ConfigManager.h"
#include "ui/nodes/UINodeLabel.h"
#include "ui/nodes/UINodeSprite.h"
#include "ui/windows/IUIMapWindow.h"
#include "ui/UI.h"

namespace caveexpress {

class HudInitDoneHandler: public InitDoneHandler {
public:
	HudInitDoneHandler (ClientMap& map) :
			InitDoneHandler(map)
	{
	}

	void execute (const InitDoneMessage* msg) override
	{
		InitDoneHandler::execute(msg);
		//  packages
		const uint8_t packages = msg->getPackages();
		UINodeLabel *pkgLeft = UI::get().getNode<UINodeLabel>(UI_WINDOW_MAP, UINODE_PACKAGES_LEFT);
		pkgLeft->setLabel(packages > 0 ? string::format("%d/%d", 0, packages) : "");
		
		UINodeSprite* pkgIcon = UI::get().getNode<UINodeSprite>(UI_WINDOW_MAP, UINODE_PACKAGES);
		pkgIcon->setVisible(packages > 0);

		//  transfers
		const uint8_t transfers = msg->getTransfers();
		UINodeLabel *npcLeft = UI::get().getNode<UINodeLabel>(UI_WINDOW_MAP, UINODE_TRANSFERS_LEFT);
		npcLeft->setLabel(transfers > 0 ? string::format("%d/%d", 0, transfers) : "");
		
		UINodeSprite* npcIcon = UI::get().getNode<UINodeSprite>(UI_WINDOW_MAP, UINODE_TRANSFERS);
		npcIcon->setVisible(transfers > 0);

		UINodeSprite* target = UI::get().getNode<UINodeSprite>(UI_WINDOW_MAP, UINODE_TARGETCAVEID);
		target->clearSprites();

		{
			const uint8_t lives = msg->getLives();
			UINodeSprite* node = UI::get().getNode<UINodeSprite>(UI_WINDOW_MAP, UINODE_LIVES);
			if (node) {
				node->clearSprites();
				const SpritePtr sprite = UI::get().loadSprite("icon-heart");
				for (uint8_t i = 0; i < lives; ++i) {
					node->addSprite(sprite);
				}
			}
		}
	}
};

}
