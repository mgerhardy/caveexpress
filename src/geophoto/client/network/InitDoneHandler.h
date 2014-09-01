#pragma once

#include "shared/network/IProtocolHandler.h"
#include "shared/network/messages/InitDoneMessage.h"
#include "shared/ConfigManager.h"

class InitDoneHandler: public IClientProtocolHandler {
public:
	InitDoneHandler ()
	{
	}

	void execute (const IProtocolMessage& message) override
	{
		const InitDoneMessage *msg = static_cast<const InitDoneMessage*>(&message);
	}
};
