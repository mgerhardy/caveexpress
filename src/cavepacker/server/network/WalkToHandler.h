#pragma once

#include "network/IProtocolHandler.h"
#include "cavepacker/server/map/Map.h"
#include "cavepacker/shared/network/messages/WalkToMessage.h"

namespace cavepacker {

class WalkToHandler: public IServerProtocolHandler {
private:
	Map& _map;
public:
	WalkToHandler (Map& map) :
			_map(map)
	{
	}

	void execute (const ClientId& clientId, const IProtocolMessage& message) override
	{
		Player* player = _map.getPlayer(clientId);
		if (player == nullptr)
			return;
		const WalkToMessage* msg = static_cast<const WalkToMessage*>(&message);
		_map.walkTo(player, msg->getCol(), msg->getRow());
	}
};

}
