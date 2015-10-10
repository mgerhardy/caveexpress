#pragma once

#include "network/IProtocolHandler.h"
#include "miniracer/server/map/Map.h"

namespace miniracer {

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
		Log::info(LOG_GAMEIMPL, "got disconnect from clientId %i", clientId);
		_map.disconnect(clientId);
	}
};

}
