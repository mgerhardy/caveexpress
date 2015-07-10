#pragma once

#include "network/IProtocolHandler.h"
#include "miniracer/server/map/Map.h"
#include "network/messages/FingerMovementMessage.h"

namespace miniracer {

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
			Log::error(LOG_SERVER, "movement for player with clientId %i failed", (int)clientId);
			return;
		}
	}
};

}
