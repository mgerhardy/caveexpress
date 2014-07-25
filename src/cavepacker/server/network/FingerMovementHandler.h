#pragma once

#include "engine/common/network/IProtocolHandler.h"
#include "cavepacker/server/map/Map.h"
#include "engine/common/network/messages/FingerMovementMessage.h"

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
			error(LOG_SERVER, "movement for player with clientId " + string::toString((int)clientId) + " failed");
			return;
		}
		const FingerMovementMessage* msg = static_cast<const FingerMovementMessage*>(&message);
		player->move(msg->getDeltaX() / std::abs(msg->getDeltaX()), msg->getDeltaY() / std::abs(msg->getDeltaY()));
	}
};
