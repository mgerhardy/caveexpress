#pragma once

#include "engine/common/network/IProtocolHandler.h"
#include "caveexpress/shared/network/messages/UpdateCollectedTypeMessage.h"

class UpdateCollectedTypeHandler: public IClientProtocolHandler {
public:
	void execute (const IProtocolMessage& message) override
	{
		// const UpdateCollectedTypeMessage *msg = static_cast<const UpdateCollectedTypeMessage*>(&message);
	}
};
