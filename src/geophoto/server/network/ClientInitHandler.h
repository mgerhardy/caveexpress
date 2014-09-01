#pragma once

#include "shared/network/IProtocolHandler.h"
#include "shared/network/messages/ClientInitMessage.h"

class ClientInitHandler: public IServerProtocolHandler {
public:
	ClientInitHandler ()
	{
	}

	void execute (const ClientId& clientId, const IProtocolMessage& message) override
	{
		const ClientInitMessage* msg = static_cast<const ClientInitMessage*>(&message);
	}
};
