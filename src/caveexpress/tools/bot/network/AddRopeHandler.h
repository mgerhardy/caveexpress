#pragma once

#include "engine/common/network/IProtocolHandler.h"
#include "caveexpress/shared/network/messages/AddRopeMessage.h"

class AddRopeHandler: public IClientProtocolHandler {
public:
	void execute (const IProtocolMessage& message) override
	{
		// const AddRopeMessage *msg = static_cast<const AddRopeMessage*>(&message);
	}
};
