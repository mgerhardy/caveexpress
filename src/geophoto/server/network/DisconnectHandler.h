#pragma once

#include "shared/network/IProtocolHandler.h"

class DisconnectHandler: public IServerProtocolHandler {
public:
	DisconnectHandler ()
	{
	}

	void execute (const ClientId& clientId, const IProtocolMessage& message) override
	{
	}
};
