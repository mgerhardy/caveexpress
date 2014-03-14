#pragma once

#include "engine/common/network/IProtocolHandler.h"
#include "engine/common/network/messages/UpdateEntityMessage.h"

class UpdateEntityHandler: public IClientProtocolHandler {
public:
	void execute (const IProtocolMessage& message) override
	{
		// const UpdateEntityMessage *msg = static_cast<const UpdateEntityMessage*>(&message);
	}
};
