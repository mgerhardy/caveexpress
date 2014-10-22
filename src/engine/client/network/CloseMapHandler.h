#pragma once

#include "engine/common/network/IProtocolHandler.h"
#include "engine/common/Logger.h"
#include "engine/client/ClientMap.h"
#include "engine/client/ui/UI.h"
#include "engine/client/ui/nodes/UINodeSprite.h"
#include "engine/client/ui/nodes/UINodePoint.h"
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
		System.track("mapstate", "close:" + _map.getName());
		info(LOG_CLIENT, "close the clientmap");
		ClientPlayer* player = _map.getPlayer();
		if (player != nullptr)
			player->setCollected(EntityType::NONE);
		UINodeSprite* collectedNode = UI::get().getNode<UINodeSprite>(UI_WINDOW_MAP, UINODE_COLLECTED);
		if (collectedNode)
			collectedNode->clearSprites();
		UINodePoint* pointsNode = UI::get().getNode<UINodePoint>(UI_WINDOW_MAP, UINODE_POINTS);
		if (pointsNode)
			pointsNode->setPoints(0);
		_map.close();
		UI::get().pop();
	}
};
