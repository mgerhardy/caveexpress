#pragma once

#include "engine/common/network/IProtocolHandler.h"
#include "caveanddiamonds/server/map/Map.h"
#include "engine/common/network/messages/StopMovementMessage.h"

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
