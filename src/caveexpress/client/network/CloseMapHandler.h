#pragma once

#include "engine/common/network/IProtocolHandler.h"
#include "caveexpress/shared/network/messages/ProtocolMessages.h"
#include "engine/common/Logger.h"
#include "engine/client/ClientMap.h"
#include "engine/client/ui/UI.h"
#include "caveexpress/client/ui/windows/UIMapWindow.h"
#include "engine/client/ui/nodes/UINodeSprite.h"
#include "engine/client/ui/nodes/UINodeLabel.h"
#include "engine/common/SpriteType.h"

class CloseMapHandler: public IClientProtocolHandler {
private:
	ClientMap& _map;
public:
	CloseMapHandler (ClientMap& map) :
			_map(map)
	{
	}

	void execute (const IProtocolMessage& message) override
	{
		if (!_map.isActive()) {
			error(LOG_CLIENT, "clientmap is not active");
			return;
		}
		System.track("MapState", "close:" + _map.getName());
		info(LOG_CLIENT, "close the clientmap");
		ClientPlayer* player = _map.getPlayer();
		if (player != nullptr)
			player->setCollected(EntityType::NONE);
		UINodeSprite* collectedNode = UI::get().getNode<UINodeSprite>(UI_WINDOW_MAP, UINODE_COLLECTED);
		collectedNode->clearSprites();
		UINodeLabel* pointsNode = UI::get().getNode<UINodeLabel>(UI_WINDOW_MAP, UINODE_POINTS);
		pointsNode->setLabel("0");
		_map.close();
		UI::get().pop();
	}
};
