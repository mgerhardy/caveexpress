#pragma once

#include "common/network/IProtocolHandler.h"
#include "common/network/messages/MapRestartMessage.h"
#include "client/ClientMap.h"

class MapRestartHandler: public IClientProtocolHandler {
private:
	ClientMap& _map;
public:
	MapRestartHandler (ClientMap& map) :
			_map(map)
	{
	}

	void execute (const IProtocolMessage& message) override
	{
		const MapRestartMessage *msg = static_cast<const MapRestartMessage*>(&message);
		_map.restart(msg->getDelay());
	}
};
