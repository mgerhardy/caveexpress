#pragma once

#include "network/IProtocolHandler.h"
#include "miniracer/server/map/Map.h"
#include "campaign/ICampaignManager.h"

namespace miniracer {

class SpawnHandler: public IServerProtocolHandler {
private:
	Map& _map;
public:
	SpawnHandler (Map& map) :
			_map(map)
	{
	}

	void execute (const ClientId& clientId, const IProtocolMessage& message) override
	{
		Player* player = new Player(_map, clientId);
		Log::info(LOG_GAMEIMPL, "spawn client %i", clientId);
		if (!_map.initPlayer(player))
			delete player;
	}
};

}
