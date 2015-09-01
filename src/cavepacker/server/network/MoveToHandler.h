#pragma once

#include <cavepacker/shared/network/messages/MoveToMessage.h>
#include "network/IProtocolHandler.h"
#include "cavepacker/server/map/Map.h"

namespace cavepacker {

class MoveToHandler: public IServerProtocolHandler {
private:
	Map& _map;
public:
	MoveToHandler (Map& map) :
			_map(map)
	{
	}

	void execute (const ClientId& clientId, const IProtocolMessage& message) override
	{
		Player* player = _map.getPlayer(clientId);
		if (player == nullptr)
			return;
		const MoveToMessage* msg = static_cast<const MoveToMessage*>(&message);
		_map.moveTo(player, msg->getCol(), msg->getRow());
	}
};

}
