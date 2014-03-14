#pragma once

#include "engine/common/network/IProtocolHandler.h"
#include "caveexpress/shared/network/messages/UpdateLivesMessage.h"

class UpdateLivesHandler: public IClientProtocolHandler {
public:
	void execute (const IProtocolMessage& message) override
	{
		// const UpdateLivesMessage *msg = static_cast<const UpdateLivesMessage*>(&message);
	}
};
