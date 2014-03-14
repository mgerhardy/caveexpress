#pragma once

#include "engine/common/network/IProtocolHandler.h"
#include "engine/common/network/messages/RemoveEntityMessage.h"

class RemoveEntityHandler: public IClientProtocolHandler {
public:
	void execute (const IProtocolMessage& message) override
	{
		// const RemoveEntityMessage *msg = static_cast<const RemoveEntityMessage*>(&message);
	}
};
