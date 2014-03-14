#pragma once

#include "engine/common/network/IProtocolHandler.h"
#include "caveexpress/shared/network/messages/WaterHeightMessage.h"

class WaterHeightHandler: public IClientProtocolHandler {
public:
	void execute (const IProtocolMessage& message) override
	{
		// const WaterHeightMessage *msg = static_cast<const WaterHeightMessage*>(&message);
	}
};

