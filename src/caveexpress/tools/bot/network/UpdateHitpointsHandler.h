#pragma once

#include "engine/common/network/IProtocolHandler.h"
#include "caveexpress/shared/network/messages/UpdateHitpointsMessage.h"

class UpdateHitpointsHandler: public IClientProtocolHandler {
public:
	void execute (const IProtocolMessage& message) override
	{
		// const UpdateHitpointsMessage *msg = static_cast<const UpdateHitpointsMessage*>(&message);
	}
};
