#pragma once

#include "engine/common/network/IProtocolHandler.h"
#include "caveanddiamonds/server/map/Map.h"
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
		_map.abortAutoSolve();
		Player* player = _map.getPlayer(clientId);
		if (player == nullptr) {
			error(LOG_SERVER, "movement for player with clientId " + string::toString((int)clientId) + " failed");
			return;
		}
		const MovementMessage* msg = static_cast<const MovementMessage*>(&message);
		switch (msg->getDirection()) {
		case DIRECTION_UP:
			_map.movePlayer(player, MOVE_UP);
			break;
		case DIRECTION_DOWN:
			_map.movePlayer(player, MOVE_DOWN);
			break;
		case DIRECTION_LEFT:
			_map.movePlayer(player, MOVE_LEFT);
			break;
		case DIRECTION_RIGHT:
			_map.movePlayer(player, MOVE_RIGHT);
			break;
		}
	}
};
