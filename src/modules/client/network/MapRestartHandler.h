#pragma once

#include "network/IProtocolHandler.h"
#include "network/messages/MapRestartMessage.h"
#include "client/ClientMap.h"

class MapRestartHandler: public ClientProtocolHandler<MapRestartMessage> {
private:
	ClientMap& _map;
public:
	MapRestartHandler (ClientMap& map) :
			_map(map)
	{
	}

	void execute (const MapRestartMessage* msg) override
	{
		_map.restart(msg->getDelay());
	}
};
