#pragma once

#include "engine/common/network/IProtocolHandler.h"

class IServerCallback {
public:
	virtual ~IServerCallback ()
	{
	}

	// called when tcp data arrives
	virtual void onData (ClientId clientId, ByteStream &data)
	{
	}

	// called when udp data arrives
	virtual ProtocolMessagePtr onOOBData (const unsigned char *data)
	{
		return ProtocolMessagePtr();
	}

	// called if a new connection was established
	virtual void onConnection (ClientId clientId)
	{
	}

	// called whenever a client disconnects
	virtual void onDisconnect (ClientId clientId)
	{
	}
};
