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
		if (_map.isAutoSolve())
			return;
		Player* player = _map.getPlayer(clientId);
		if (player == nullptr) {
			error(LOG_SERVER, "movement for player with clientId " + string::toString((int)clientId) + " failed");
			return;
		}
		const MovementMessage* msg = static_cast<const MovementMessage*>(&message);
		switch (msg->getDirection()) {
		case DIRECTION_UP:
			player->moveByChar('u');
			break;
		case DIRECTION_DOWN:
			player->moveByChar('d');
			break;
		case DIRECTION_LEFT:
			player->moveByChar('l');
			break;
		case DIRECTION_RIGHT:
			player->moveByChar('r');
			break;
		}
	}
};
