#pragma once

#include "common/ByteStream.h"
#include "IProtocolHandler.h"
#include "common/Log.h"
#include "IClientCallback.h"
#include "IServerCallback.h"
#include "ProtocolMessageFactory.h"
#include <map>
#include <string>

#define ClientIdToClientMask(clientId) (1 << clientId)

#define MAX_CLIENTS 4

namespace {
const ClientId defaultClientId = 1;
}

class INetwork {
protected:
	IClientCallback* _clientFunc;
	IServerCallback* _serverFunc;

	typedef std::map<protocolId, uint32_t> CountMap;
	static CountMap _count;

	void count (const IProtocolMessage &msg);
public:
	INetwork ();

	virtual ~INetwork ();

	virtual void init () {}
	virtual void shutdown ();

	// this handles the sending and the receiving of the data from the networks channel(s).
	// this method does not block
	virtual void update (uint32_t deltaTime) = 0;

	// open a new listen socket on the given port and close any other server socket
	// that is maybe still opened
	virtual bool openServer (int port, IServerCallback* func) = 0;

	// send a message to connected clients
	// @param[in] clientMask If @c 0, all connected clients will get the message.
	// @return the amount of clients the message was sent to
	virtual int sendToClients (int clientMask, const IProtocolMessage& msg) = 0;

	virtual int sendToAllClients (const IProtocolMessage& msg);

	virtual int sendToClient (ClientId clientId, const IProtocolMessage& msg)
	{
		const int clients = sendToClients(ClientIdToClientMask(clientId), msg);
		if (clients != 1) {
			if (clients == 0)
				Log::error2(LOG_NET, "message with the id %i wasn't sent to the client", (int)msg.getId());
			else
				Log::error2(LOG_NET, "message with the id %i was send to multiple clients (%i)", (int)msg.getId(), clients);
		}
		return clients;
	}

	virtual bool discover (IClientCallback* callback, int port)
	{
		const char *buf = "discover";
		const size_t length = strlen(buf) + 1;
		return broadcast(callback, const_cast<uint8_t*>((const uint8_t*) buf), length, port);
	}

	virtual void closeServer () = 0;

	// disconnects a client from the server
	virtual void disconnectClientFromServer (ClientId clientId) = 0;

	virtual bool isServer () const = 0;
	virtual bool isClient () const = 0;
	virtual bool isMultiplayer () const { return true; }

	virtual bool openClient (const std::string& node, int port, IClientCallback* func) = 0;

	// the client is sending data to the server
	virtual int sendToServer (const IProtocolMessage& msg) = 0;

	virtual void closeClient () = 0;

	// is the client connected
	virtual bool isClientConnected () = 0;

	virtual bool broadcast (IClientCallback* oobCallback, uint8_t* buffer, size_t length, int port) = 0;
};
