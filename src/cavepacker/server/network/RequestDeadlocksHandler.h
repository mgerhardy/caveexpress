#pragma once

#include "network/IProtocolHandler.h"
#include "cavepacker/server/map/Map.h"

namespace cavepacker {

class RequestDeadlocksHandler: public IServerProtocolHandler {
private:
	Map& _map;
public:
	explicit RequestDeadlocksHandler (Map& map) :
			_map(map)
	{
	}

	void execute (const ClientId& clientId, const IProtocolMessage& message) override
	{
		_map.sendDeadlocks(clientId);
	}
};

}
