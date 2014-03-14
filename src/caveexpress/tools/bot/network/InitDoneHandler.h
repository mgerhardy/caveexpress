#pragma once

#include "engine/common/network/IProtocolHandler.h"
#include "engine/common/network/messages/InitDoneMessage.h"

class InitDoneHandler: public IClientProtocolHandler {
public:
	void execute (const IProtocolMessage& message) override
	{
		// const InitDoneMessage *msg = static_cast<const InitDoneMessage*>(&message);
	}
};
