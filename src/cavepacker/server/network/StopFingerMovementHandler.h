#pragma once

#include "network/IProtocolHandler.h"
#include "cavepacker/server/map/Map.h"
#include "network/messages/FingerMovementMessage.h"

namespace cavepacker {

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
			Log::error(LOG_SERVER, "movement for player with clientId " + string::toString((int)clientId) + " failed");
			return;
		}
	}
};

}
