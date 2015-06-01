#pragma once

#include "IProtocolHandler.h"
#include <string>

class IClientCallback {
public:
	virtual ~IClientCallback ()
	{
	}

	// called when tcp data arrives
	virtual void onData (ByteStream &data)
	{
	}

	virtual void onConnectionError ()
	{
	}

	virtual void onConnectionSuccess ()
	{
	}

	// called when udp data arrives
	virtual void onOOBData (const std::string& host, const IProtocolMessage* message)
	{
	}
};
