#pragma once

#include "engine/common/network/IProtocolHandler.h"
#include "caveexpress/shared/network/messages/ProtocolMessages.h"

class CloseMapHandler: public IClientProtocolHandler {
public:
	void execute (const IProtocolMessage& message) override
	{
	}
};
