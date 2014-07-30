#pragma once

#include "engine/common/network/IProtocolHandler.h"
#include "cavepacker/server/map/Map.h"
#include "engine/common/campaign/ICampaignManager.h"

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
		info(LOG_SERVER, String::format("spawn client %i", clientId));
		if (!_map.initPlayer(player))
			delete player;
	}
};
