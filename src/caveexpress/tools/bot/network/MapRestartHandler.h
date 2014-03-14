#pragma once

#include "engine/common/network/IProtocolHandler.h"
#include "engine/common/network/messages/MapRestartMessage.h"

class MapRestartHandler: public IClientProtocolHandler {
public:
	void execute (const IProtocolMessage& message) override
	{
		// const MapRestartMessage *msg = static_cast<const MapRestartMessage*>(&message);
	}
};
