#include "NoNetwork.h"
#include "engine/common/Logger.h"
#include <assert.h>

void NoNetwork::update (uint32_t deltaTime)
{
	if (_clientFunc != nullptr) {
		for (QueueIter i = _clientQueue.begin(); i != _clientQueue.end(); ++i) {
			_clientFunc->onData(*i);
			// there might be a drop
			if (_clientFunc == nullptr)
				break;
		}
		_clientQueue.clear();
		_clientQueue.reserve(64);
	}

	if (_serverFunc != nullptr) {
		for (QueueIter i = _serverQueue.begin(); i != _serverQueue.end(); ++i) {
			_serverFunc->onData(defaultClientId, *i);
			// there might be a drop
			if (_serverFunc == nullptr)
				break;
		}
		_serverQueue.clear();
		_serverQueue.reserve(64);
	}
}

bool NoNetwork::openServer (int port, IServerCallback* func)
{
	info(LOG_NET, String::format("open server on port %i", port));
	_serverFunc = func;
	_server = true;
	return true;
}

int NoNetwork::sendToClients (int clientMask, const IProtocolMessage& msg)
{
	debug(LOG_NET, String::format("send to client message type %i", msg.getId()));
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

	info(LOG_NET, "close server");
	_server = false;
	_serverFunc = nullptr;
}

void NoNetwork::disconnectClientFromServer (ClientId clientId)
{
	if (!isClientConnected())
		return;
	info(LOG_NET, "disconnect client");
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
	info(LOG_NET, "init the network layer (local)");
}

bool NoNetwork::openClient (const std::string& node, int port, IClientCallback* func)
{
	if (isClientConnected())
		return false;

	info(LOG_NET, String::format("connect to %s:%i", node.c_str(), port));
	closeClient();
	_clientFunc = func;
	assert(func);
	_clientQueue.clear();
	_serverQueue.clear();

	_clientQueue.reserve(64);
	_serverQueue.reserve(64);

	if (_clientFunc != nullptr && !_connected) {
		info(LOG_NET, String::format("connect %i", defaultClientId));
		_serverFunc->onConnection(defaultClientId);
		_connected = true;
	}

	return true;
}

int NoNetwork::sendToServer (const IProtocolMessage& msg)
{
	debug(LOG_NET, String::format("send to server message type %i", msg.getId()));
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

	info(LOG_NET, "close client");
	const DisconnectMessage msg;
	sendToServer(msg);
	_clientFunc = nullptr;

	if (_connected) {
		_serverFunc->onDisconnect(defaultClientId);
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
	return false;
}
