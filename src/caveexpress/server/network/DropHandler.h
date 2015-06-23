#pragma once

#include "network/IProtocolHandler.h"
#include "caveexpress/server/map/Map.h"

namespace caveexpress {

class DropHandler: public IServerProtocolHandler {
private:
	Map& _map;
public:
	DropHandler (Map& map) :
			_map(map)
	{
	}

	void execute (const ClientId& clientId, const IProtocolMessage& message) override
	{
		Player* player = _map.getPlayer(clientId);
		if (player == nullptr) {
			Log::error2(LOG_SERVER, "drop for player with clientId %i failed", (int)clientId);
			return;
		}
		player->drop();
	}
};

}
