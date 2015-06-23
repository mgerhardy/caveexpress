#pragma once

#include "network/IProtocolHandler.h"
#include "common/Log.h"
#include "client/ClientMap.h"
#include "ui/UI.h"
#include "ui/nodes/UINodeSprite.h"
#include "ui/nodes/UINodePoint.h"
#include "common/SpriteType.h"

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
			Log::error(LOG_CLIENT, "clientmap is not active");
			return;
		}
		System.track("mapstate", "close:" + _map.getName());
		Log::info(LOG_CLIENT, "close the clientmap");
		ClientPlayer* player = _map.getPlayer();
		if (player != nullptr)
			player->setCollected(EntityType::NONE);
		UINodeSprite* collectedNode = UI::get().getNode<UINodeSprite>(UI_WINDOW_MAP, UINODE_COLLECTED);
		if (collectedNode)
			collectedNode->clearSprites();
		UINodeSprite* targetCaveNode = UI::get().getNode<UINodeSprite>(UI_WINDOW_MAP, UINODE_TARGETCAVEID);
		if (targetCaveNode)
			targetCaveNode->clearSprites();
		UINodePoint* pointsNode = UI::get().getNode<UINodePoint>(UI_WINDOW_MAP, UINODE_POINTS);
		if (pointsNode)
			pointsNode->setPoints(0);
		_map.close();
		UI::get().pop();
	}
};
