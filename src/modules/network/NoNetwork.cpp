#include "NoNetwork.h"
#include "common/Log.h"
#include <assert.h>

void NoNetwork::update (uint32_t deltaTime)
{
	if (_clientFunc != nullptr) {
		const Queue q = _clientQueue;
		_clientQueue.clear();
		_clientQueue.reserve(64);
		for (QueueConstIter i = q.begin(); i != q.end(); ++i) {
			ByteStream b = *i;
			_clientFunc->onData(b);
			// there might be a drop
			if (_clientFunc == nullptr)
				break;
		}
	}

	if (_serverFunc != nullptr) {
		const Queue q = _serverQueue;
		_serverQueue.clear();
		_serverQueue.reserve(64);
		for (QueueConstIter i = q.begin(); i != q.end(); ++i) {
			ByteStream b = *i;
			_serverFunc->onData(defaultClientId, b);
			// there might be a drop
			if (_serverFunc == nullptr)
				break;
		}
	}
}

bool NoNetwork::openServer (int port, IServerCallback* func)
{
	Log::info2(LOG_NET, "open server on port %i", port);
	_serverFunc = func;
	_server = true;
	return true;
}

int NoNetwork::sendToClients (int clientMask, const IProtocolMessage& msg)
{
	Log::trace(LOG_NET, "send to client message type %i", msg.getId());
	ByteStream s;
	msg.serialize(s);
	s.addShort(s.getSize(), true);
	count(msg);
	enqueueClient(s);
	return 1;
}

void NoNetwork::closeServer ()
{
	if (!isServer())
		return;

	closeClient();

	_clientQueue.clear();
	_clientQueue.reserve(64);
	_serverQueue.clear();
	_serverQueue.reserve(64);

	Log::info(LOG_NET, "close server");
	_server = false;
	_serverFunc = nullptr;
}

void NoNetwork::disconnectClientFromServer (ClientId clientId)
{
	if (!isClientConnected())
		return;
	Log::info(LOG_NET, "disconnect client");
}

bool NoNetwork::isServer () const
{
	return _server;
}

bool NoNetwork::isClient () const
{
	return _connected;
}

void NoNetwork::init ()
{
	Log::info(LOG_NET, "init the network layer (local)");
}

bool NoNetwork::openClient (const std::string& node, int port, IClientCallback* func)
{
	if (isClientConnected())
		return false;

	if (!isServer())
		return false;

	Log::info2(LOG_NET, "connect to %s:%i", node.c_str(), port);
	closeClient();
	_clientFunc = func;
	assert(func);
	_clientQueue.clear();
	_serverQueue.clear();

	_clientQueue.reserve(64);
	_serverQueue.reserve(64);

	if (_clientFunc != nullptr && !_connected) {
		Log::info2(LOG_NET, "connect %i", defaultClientId);
		if (_serverFunc)
			_serverFunc->onConnection(defaultClientId);
		_connected = true;
	}

	return true;
}

int NoNetwork::sendToServer (const IProtocolMessage& msg)
{
	Log::trace(LOG_NET, "send to server message type %i", msg.getId());
	ByteStream s;
	msg.serialize(s);
	s.addShort(s.getSize(), true);
	count(msg);
	enqueueServer(s);
	return s.getSize();
}

void NoNetwork::closeClient ()
{
	if (!isClientConnected())
		return;

	Log::info(LOG_NET, "close client");
	const DisconnectMessage msg;
	sendToServer(msg);
	_clientFunc = nullptr;

	if (_connected) {
		if (_serverFunc) {
			_serverFunc->onDisconnect(defaultClientId);
		}
		_connected = false;
	}

	_clientQueue.clear();
	_serverQueue.clear();
}

bool NoNetwork::isClientConnected ()
{
	return _connected;
}

bool NoNetwork::broadcast (IClientCallback* oobCallback, uint8_t* buffer, size_t length, int port)
{
	Log::error(LOG_NET, "local network doesn't support broadcasting");
	return false;
}
