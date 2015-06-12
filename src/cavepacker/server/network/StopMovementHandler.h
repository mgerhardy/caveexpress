#pragma once

#include "network/IProtocolHandler.h"
#include "cavepacker/server/map/Map.h"
#include "network/messages/StopMovementMessage.h"

namespace cavepacker {

class StopMovementHandler: public IServerProtocolHandler {
private:
public:
	StopMovementHandler (Map& map)
	{
	}

	void execute (const ClientId& clientId, const IProtocolMessage& message) override
	{
	}
};

}
