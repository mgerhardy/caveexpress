#pragma once

#include "network/IProtocolHandler.h"
#include "cavepacker/server/map/Map.h"
#include "network/messages/StopMovementMessage.h"

namespace cavepacker {

class StopMovementHandler: public IServerProtocolHandler {
private:
	Map& _map;
public:
	StopMovementHandler (Map& map) :
			_map(map)
	{
	}

	void execute (const ClientId& clientId, const IProtocolMessage& message) override
	{
		Player* player = _map.getPlayer(clientId);
		if (player == nullptr)
			return;
		const StopMovementMessage* msg = static_cast<const StopMovementMessage*>(&message);
		player->clearHeldDirection(msg->getDirection());
	}
};

}
