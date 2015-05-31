#pragma once

#include "common/network/IProtocolHandler.h"
#include "cavepacker/server/map/Map.h"
#include "common/network/messages/FingerMovementMessage.h"

class FingerMovementHandler: public IServerProtocolHandler {
private:
#if 0
	Map& _map;
#endif
public:
	FingerMovementHandler (Map& map)
#if 0
 :
			_map(map)
#endif
	{
	}

	void execute (const ClientId& clientId, const IProtocolMessage& message) override
	{
#if 0
		Player* player = _map.getPlayer(clientId);
		if (player == nullptr) {
			error(LOG_SERVER, "movement for player with clientId " + string::toString((int)clientId) + " failed");
			return;
		}
		const FingerMovementMessage* msg = static_cast<const FingerMovementMessage*>(&message);
		const int dx = msg->getDeltaX() == 0 ? 0 : msg->getDeltaX() / std::abs(msg->getDeltaX());
		const int dy = msg->getDeltaY() == 0 ? 0 : msg->getDeltaY() / std::abs(msg->getDeltaY());
		_map.movePlayer(player, MOVE_DOWN); // TODO
#endif
	}
};
