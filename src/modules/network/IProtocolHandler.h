#pragma once

#include "common/ByteStream.h"
#include "common/Pointers.h"
#include "IProtocolMessage.h"

typedef uint8_t ClientId;

class IClientProtocolHandler {
public:
	virtual ~IClientProtocolHandler ()
	{
	}

	virtual void execute (const IProtocolMessage& message) = 0;
};

class IServerProtocolHandler {
public:
	virtual ~IServerProtocolHandler ()
	{
	}

	virtual void execute (const ClientId& clientId, const IProtocolMessage& message) = 0;
};
