#pragma once

#include "shared/network/IProtocolHandler.h"
#include "server/map/Map.h"
#include "shared/campaign/persister/IGameStatePersister.h"

class SpawnHandler: public IServerProtocolHandler {
private:
	Map& _map;
	IGameStatePersister& _persister;
public:
	SpawnHandler (Map& map, IGameStatePersister& persister) :
			_map(map), _persister(persister)
	{
	}

	void execute (const ClientId& clientId, const IProtocolMessage& message) override
	{
		// add the new player to every connected player
		Player* player = new Player(_map, clientId, _persister);
		const std::string& activeCampaign = _persister.getActiveCampaign();
		const uint8_t lives = _persister.getLives(activeCampaign);
		player->setLives(lives);
		info(LOG_SERVER, String::format("spawn client %i with %i lives", clientId, player->getLives()));
		_map.initPlayer(player);
	}
};
