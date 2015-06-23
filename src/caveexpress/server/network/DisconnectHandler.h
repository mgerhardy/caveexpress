#pragma once

#include "network/IProtocolHandler.h"
#include "caveexpress/server/map/Map.h"

namespace caveexpress {

class DisconnectHandler: public IServerProtocolHandler {
private:
	Map& _map;
public:
	DisconnectHandler (Map& map) :
			_map(map)
	{
	}

	void execute (const ClientId& clientId, const IProtocolMessage& message) override
	{
		Log::info(LOG_SERVER, "got disconnect from clientId %i", clientId);
		_map.disconnect(clientId);
	}
};

}
