#pragma once

#include "engine/common/network/IProtocolHandler.h"
#include "caveexpress/shared/network/messages/AddCaveMessage.h"

class AddCaveHandler: public IClientProtocolHandler {
public:
	void execute (const IProtocolMessage& message) override
	{
		// const AddCaveMessage *msg = static_cast<const AddCaveMessage*>(&message);
	}
};
