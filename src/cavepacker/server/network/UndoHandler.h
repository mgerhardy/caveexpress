#pragma once

#include "network/IProtocolHandler.h"
#include "cavepacker/server/map/Map.h"

namespace cavepacker {

class UndoHandler: public IServerProtocolHandler {
private:
	Map& _map;
public:
	UndoHandler (Map& map) :
			_map(map)
	{
	}

	void execute (const ClientId& clientId, const IProtocolMessage& message) override
	{
		Player* player = _map.getPlayer(clientId);
		if (player == nullptr || player->isSpectator())
			return;
		_map.undo(player);
	}
};

}
