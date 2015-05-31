#pragma once

#include "common/network/IProtocolHandler.h"
#include "cavepacker/server/map/Map.h"
#include "common/network/messages/StopMovementMessage.h"

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
