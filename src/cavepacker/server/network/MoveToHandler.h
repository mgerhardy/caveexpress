#pragma once

#include <cavepacker/shared/network/messages/MoveToMessage.h>
#include "network/IProtocolHandler.h"
#include "cavepacker/server/map/Map.h"

namespace cavepacker {

class MoveToHandler: public IServerProtocolHandler {
private:
	Map& _map;
public:
	MoveToHandler (Map& map) :
			_map(map)
	{
	}

	void execute (const ClientId& clientId, const IProtocolMessage& message) override
	{
		Player* player = _map.getPlayer(clientId);
		if (player == nullptr)
			return;
		const MoveToMessage* msg = static_cast<const MoveToMessage*>(&message);
		const int col = msg->getCol();
		const int row = msg->getRow();
		const BoardState& state = _map.getBoardState();
		if (col < 0 || col >= state.getWidth())
			return;
		if (row < 0 || row >= state.getHeight())
			return;

		_map.moveTo(player, col, row);
	}
};

}
