#pragma once

#include "network/INetwork.h"
#include "network/ProtocolHandlerRegistry.h"

class NetworkTestServerListener: public IServerCallback {
public:
	NetworkTestServerListener() :
			_count(0), _errorCount(0), _lastError("") {
	}
	int _count;
	int _errorCount;
	std::string _lastError;
	void onData (ClientId clientId, ByteStream &data) override
	{
		ProtocolMessageFactory& factory = ProtocolMessageFactory::get();
		while (factory.isNewMessageAvailable(data)) {
			++_count;
			// remove the size from the stream
			data.readShort();
			IProtocolMessage* msg(factory.createMsg(data));
			if (!msg) {
				_errorCount++;
				_lastError = "no message for type " + string::toString(static_cast<int>(data.readByte()));
				error(LOG_NET, "no message for type " + string::toString(static_cast<int>(data.readByte())));
				continue;
			}
			IServerProtocolHandler* handler = ProtocolHandlerRegistry::get().getServerHandler(*msg);
			if (handler == nullptr) {
				_errorCount++;
				_lastError = "no server handler for message";
				error(LOG_NET, String::format("no server handler for message type %i", msg->getId()));
				continue;
			}
			handler->execute(clientId, *msg);
		}
	}

	void onConnection (ClientId clientId) override
	{
		info(LOG_SERVER, "connect of client with id " + string::toString(static_cast<int>(clientId)));
		Singleton<GameRegistry>::getInstance().getGame()->connect(clientId);
	}

	void onDisconnect (ClientId clientId) override
	{
		info(LOG_SERVER, "disconnect of client with id " + string::toString(static_cast<int>(clientId)));
		const GamePtr& game = Singleton<GameRegistry>::getInstance().getGame();
		if (game->disconnect(clientId) == 0) {
			game->mapShutdown();
		}
	}
};

class NetworkTestListener: public IClientCallback {
public:
	NetworkTestListener() :
			_count(0), _errorCount(0), _lastError("") {
	}
	int _count;
	int _errorCount;
	std::string _lastError;
	void onData (ByteStream& data) {
		ProtocolMessageFactory& factory = ProtocolMessageFactory::get();
		while (factory.isNewMessageAvailable(data)) {
			++_count;
			// remove the size from the stream
			data.readShort();
			IProtocolMessage* msg(factory.createMsg(data));
			if (!msg) {
				_errorCount++;
				_lastError = "no message for type " + string::toString(static_cast<int>(data.readByte()));
				continue;
			}

			debug(LOG_NET, String::format("received message type %i", msg->getId()));
			IClientProtocolHandler* handler = ProtocolHandlerRegistry::get().getClientHandler(*msg);
			if (handler == nullptr) {
				_errorCount++;
				_lastError = "no client handler for message";
				error(LOG_NET, String::format("no client handler for message type %i", msg->getId()));
				continue;
			}
			handler->execute(*msg);
		}
	}
};

