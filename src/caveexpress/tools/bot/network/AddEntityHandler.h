#pragma once

#include "engine/common/network/IProtocolHandler.h"
#include "engine/common/network/messages/AddEntityMessage.h"

class AddEntityHandler: public IClientProtocolHandler {
public:
	void execute (const IProtocolMessage& message) override
	{
		// const AddEntityMessage *msg = static_cast<const AddEntityMessage*>(&message);
	}
};
