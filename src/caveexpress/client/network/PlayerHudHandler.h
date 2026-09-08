#pragma once

#include "network/IProtocolHandler.h"
#include "caveexpress/shared/network/messages/PlayerHudMessage.h"
#include "caveexpress/client/CaveExpressClientMap.h"

namespace caveexpress {

class PlayerHudHandler: public ClientProtocolHandler<PlayerHudMessage> {
private:
	CaveExpressClientMap& _map;
public:
	PlayerHudHandler (CaveExpressClientMap& map) :
			_map(map)
	{
	}

	void execute (const PlayerHudMessage* msg) override
	{
		_map.storePlayerHud(msg->getEntityId(), msg->getHitpoints(), msg->getLives(),
				msg->getTargetCave(), msg->getCollectedTypeId());
		if (_map.isLocalPlayerSpectating())
			_map.applyFollowedPlayerHudIfChanged();
	}
};

}
