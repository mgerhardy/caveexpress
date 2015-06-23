#pragma once

#include "network/IProtocolHandler.h"
#include "network/messages/ClientInitMessage.h"
#include "cavepacker/server/map/Map.h"

namespace cavepacker {

class ClientInitHandler: public IServerProtocolHandler {
private:
	Map& _map;
public:
	ClientInitHandler (Map& map) :
			_map(map)
	{
	}

	void execute (const ClientId& clientId, const IProtocolMessage& message) override
	{
		Player *player = _map.getPlayer(clientId);
		if (player == nullptr) {
			Log::error2(LOG_SERVER, "init for player with clientId %i failed", (int)clientId);
			return;
		}
		const ClientInitMessage* msg = static_cast<const ClientInitMessage*>(&message);
		player->setName(msg->getName());
		_map.sendPlayersList();
	}
};

}
