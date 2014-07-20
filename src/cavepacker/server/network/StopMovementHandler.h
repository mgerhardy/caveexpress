#pragma once

#include "engine/common/network/IProtocolHandler.h"
#include "cavepacker/server/map/Map.h"
#include "engine/common/network/messages/StopMovementMessage.h"

class StopMovementHandler: public IServerProtocolHandler {
private:
	Map& _map;
public:
	StopMovementHandler (Map& map) :
			_map(map)
	{
	}

	void execute (const ClientId& clientId, const IProtocolMessage& message) override
	{
	}
};
