#pragma once

#include "network/IProtocolHandler.h"
#include "caveexpress/shared/network/messages/TargetCaveMessage.h"
#include "client/ClientMap.h"
#include "ui/UI.h"
#include "caveexpress/client/entities/ClientNPC.h"
#include "caveexpress/client/ui/windows/UIMapWindow.h"
#include "ui/nodes/UINodeLabel.h"
#include "ui/nodes/UINodeSprite.h"
#include "common/String.h"
#include <string>

namespace caveexpress {

class TargetCaveHandler: public ClientProtocolHandler<TargetCaveMessage> {
public:
	TargetCaveHandler() {
	}

	void execute(const TargetCaveMessage* msg) override
	{
		const float ang = msg->getAngle();
		UINodeSpriteRot* arrow = UI::get().getNode<UINodeSpriteRot>(UI_WINDOW_MAP, UINODE_TARGET_ARROW);
		arrow->_angle = ang;

		const uint8_t caveNumber = msg->getCaveNumber();
		if (caveNumber >= 200)  // from update
			return;
		
		UINodeSprite* node = UI::get().getNode<UINodeSprite>(UI_WINDOW_MAP, UINODE_TARGETCAVEID);
		if (caveNumber == 0) {
			node->clearSprites();
			return;
		}
		if (caveNumber >= 100) {
			const SpritePtr& sprite = UI::get().loadSprite("item-stone-idle");
			node->addSprite(sprite);
			return;
		}

		const std::string caveNumberStr = "cavenumber" + string::toString((int) caveNumber);
		const SpritePtr& sprite = UI::get().loadSprite(caveNumberStr);
		node->addSprite(sprite);
	}
};

}
