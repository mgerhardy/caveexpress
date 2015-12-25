#pragma once

#include "INetwork.h"

class NoNetwork : public INetwork {
private:
	bool _connected;
	bool _server;

	typedef std::vector<ByteStream> Queue;
	typedef Queue::const_iterator QueueConstIter;
	typedef Queue::iterator QueueIter;
	Queue _serverQueue;
	Queue _clientQueue;

	inline void enqueueServer (const ByteStream& s)
	{
		_serverQueue.push_back(s);
	}

	inline void enqueueClient (const ByteStream& s)
	{
		_clientQueue.push_back(s);
	}

public:
	NoNetwork () :
			INetwork(), _connected(false), _server(false)
	{
	}

	virtual ~NoNetwork ()
	{
	}

	void init () override;
	void shutdown () override;
	void update (uint32_t deltaTime) override;
	bool openServer (int port, IServerCallback* func) override;
	int sendToClients (int clientMask, const IProtocolMessage& msg) override;
	void closeServer () override;
	void disconnectClientFromServer (ClientId clientId) override;
	bool isServer () const override;
	bool isClient () const override;
	bool isMultiplayer () const override { return false; }
	bool openClient (const std::string& node, int port, IClientCallback* func) override;
	int sendToServer (const IProtocolMessage& msg) override;
	void closeClient () override;
	bool isClientConnected () override;
	bool broadcast (IClientCallback* oobCallback, uint8_t* buffer, size_t length, int port) override;
};
