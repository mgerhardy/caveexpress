#pragma once

#include "common/ByteStream.h"
#include <memory>
#include "IProtocolMessage.h"

typedef uint8_t ClientId;

class IClientProtocolHandler {
public:
	virtual ~IClientProtocolHandler ()
	{
	}

	virtual void execute (const IProtocolMessage& message) = 0;
};

template<class T>
class ClientProtocolHandler : public IClientProtocolHandler {
public:
	virtual ~ClientProtocolHandler ()
	{
	}

	void execute (const IProtocolMessage& message) override {
		const T *msg = assert_cast<const T*>(&message);
		execute(msg);
	}

	virtual void execute (const T* message) = 0;
};

class IServerProtocolHandler {
public:
	virtual ~IServerProtocolHandler ()
	{
	}

	virtual void execute (const ClientId& clientId, const IProtocolMessage& message) = 0;
};
