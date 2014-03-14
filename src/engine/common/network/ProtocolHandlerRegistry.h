#pragma once

#include "IProtocolHandler.h"
#include "engine/common/NonCopyable.h"
#include <map>

class ProtocolHandlerRegistry: public NonCopyable {
private:
	typedef std::map<protocolId, IClientProtocolHandler*> ClientProtocolHandlers;
	ClientProtocolHandlers _clientRegistry;
	typedef std::map<protocolId, IServerProtocolHandler*> ServerProtocolHandlers;
	ServerProtocolHandlers _serverRegistry;

	ProtocolHandlerRegistry ()
	{
	}

public:
	static ProtocolHandlerRegistry& get ()
	{
		static ProtocolHandlerRegistry _instance;
		return _instance;
	}

	virtual ~ProtocolHandlerRegistry ()
	{
		for (ClientProtocolHandlers::iterator i = _clientRegistry.begin(); i != _clientRegistry.end(); ++i) {
			delete i->second;
		}
		for (ServerProtocolHandlers::iterator i = _serverRegistry.begin(); i != _serverRegistry.end(); ++i) {
			delete i->second;
		}
	}

	void registerClientHandler (protocolId type, IClientProtocolHandler* handler)
	{
		_clientRegistry.insert(std::make_pair(type, handler));
	}

	void unregisterClientHandler (protocolId type)
	{
		ClientProtocolHandlers::iterator i = _clientRegistry.find(type);
		if (i == _clientRegistry.end())
			return;
		delete i->second;
		_clientRegistry.erase(i);
	}

	inline IClientProtocolHandler* getClientHandler (const IProtocolMessage& msg)
	{
		ClientProtocolHandlers::iterator i = _clientRegistry.find(msg.getId());
		if (i != _clientRegistry.end())
			return i->second;

		return nullptr;
	}

	inline void registerServerHandler (protocolId type, IServerProtocolHandler* handler)
	{
		_serverRegistry.insert(std::make_pair(type, handler));
	}

	inline IServerProtocolHandler* getServerHandler (const IProtocolMessage& msg)
	{
		ServerProtocolHandlers::iterator i = _serverRegistry.find(msg.getId());
		if (i != _serverRegistry.end())
			return i->second;

		return nullptr;
	}
};
