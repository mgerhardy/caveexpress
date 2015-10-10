#pragma once

#include "network/IProtocolHandler.h"
#include "caveexpress/server/map/Map.h"
#include "network/messages/MovementMessage.h"

namespace caveexpress {

class MovementHandler: public IServerProtocolHandler {
private:
	Map& _map;
public:
	MovementHandler (Map& map) :
			_map(map)
	{
	}

	void execute (const ClientId& clientId, const IProtocolMessage& message) override
	{
		Player* player = _map.getPlayer(clientId);
		if (player == nullptr) {
			Log::error(LOG_GAMEIMPL, "movement for player with clientId %i failed", (int)clientId);
			return;
		}
		const MovementMessage* msg = static_cast<const MovementMessage*>(&message);
		player->accelerate(msg->getDirection());
	}
};

}
