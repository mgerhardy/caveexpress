#pragma once

#include "network/IProtocolHandler.h"
#include "caveexpress/server/map/Map.h"
#include "network/messages/FingerMovementMessage.h"

namespace caveexpress {

class FingerMovementHandler: public IServerProtocolHandler {
private:
	Map& _map;
public:
	FingerMovementHandler (Map& map) :
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
		const FingerMovementMessage* msg = static_cast<const FingerMovementMessage*>(&message);
		player->setAcceleration(msg->getDeltaX(), msg->getDeltaY());
	}
};

}
