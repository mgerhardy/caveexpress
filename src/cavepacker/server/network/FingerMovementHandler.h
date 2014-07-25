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
		const int dx = msg->getDeltaX() == 0 ? 0 : msg->getDeltaX() / std::abs(msg->getDeltaX());
		const int dy = msg->getDeltaY() == 0 ? 0 : msg->getDeltaY() / std::abs(msg->getDeltaY());
		player->move(dx, dy);
	}
};
