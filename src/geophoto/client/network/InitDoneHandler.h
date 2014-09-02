#pragma once

#include "engine/common/network/IProtocolHandler.h"
#include "engine/common/network/messages/InitDoneMessage.h"
#include "engine/common/ConfigManager.h"

class InitDoneHandler: public IClientProtocolHandler {
public:
	InitDoneHandler ()
	{
	}

	void execute (const IProtocolMessage& message) override
	{
	}
};
