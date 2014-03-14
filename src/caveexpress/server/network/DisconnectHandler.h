#pragma once

#include "engine/common/network/IProtocolHandler.h"
#include "caveexpress/server/map/Map.h"

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
		info(LOG_SERVER, String::format("got disconnect from clientId %i", clientId));
		_map.disconnect(clientId);
	}
};
