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
			Log::error(LOG_GAMEIMPL, "movement for player with clientId %i failed", (int)clientId);
			return;
		}
		const FingerMovementMessage* msg = assert_cast<const FingerMovementMessage*, const IProtocolMessage*>(&message);
		player->setFingerAcceleration(msg->getDeltaX(), msg->getDeltaY());
	}
};

}
