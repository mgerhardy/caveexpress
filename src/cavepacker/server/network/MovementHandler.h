#pragma once

#include "engine/common/network/IProtocolHandler.h"
#include "cavepacker/server/map/Map.h"
#include "engine/common/network/messages/MovementMessage.h"

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
			error(LOG_SERVER, "movement for player with clientId " + string::toString((int)clientId) + " failed");
			return;
		}
		const MovementMessage* msg = static_cast<const MovementMessage*>(&message);
		switch (msg->getDirection()) {
		case DIRECTION_UP:
			player->move(0, -1);
			break;
		case DIRECTION_DOWN:
			player->move(0, 1);
			break;
		case DIRECTION_LEFT:
			player->move(-1, 0);
			break;
		case DIRECTION_RIGHT:
			player->move(1, 0);
			break;
		}
	}
};
