#pragma once

#include "network/IProtocolHandler.h"
#include "caveexpress/server/map/Map.h"
#include "network/messages/FingerMovementMessage.h"

namespace caveexpress {

class StopFingerMovementHandler: public IServerProtocolHandler {
private:
	Map& _map;
public:
	StopFingerMovementHandler (Map& map) :
			_map(map)
	{
	}

	void execute (const ClientId& clientId, const IProtocolMessage& message) override
	{
		Player* player = _map.getPlayer(clientId);
		if (player == nullptr) {
			error(LOG_SERVER, "movement for player with clientId " + string::toString((int)clientId) + " failed");
			return;
		}
		player->resetAcceleration();
	}
};

}
