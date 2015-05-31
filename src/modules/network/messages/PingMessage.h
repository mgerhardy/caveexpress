#pragma once

#include "network/IProtocolMessage.h"
#include <string>

class PingMessage: public IProtocolMessage {
private:
	std::string _name;
	std::string _mapName;
	int _port;
	int _playerCount;
	int _maxPlayerCount;

public:
	PingMessage (const std::string& name, const std::string& mapName, int port, int playerCount, int maxPlayerCount) :
			IProtocolMessage(protocol::PROTO_PING), _name(name), _mapName(mapName), _port(port), _playerCount(playerCount), _maxPlayerCount(
					maxPlayerCount)
	{
	}

	PingMessage (ByteStream& input) :
			IProtocolMessage(protocol::PROTO_PING)
	{
		_name = input.readString();
		_mapName = input.readString();
		_port = input.readInt();
		_playerCount = input.readByte();
		_maxPlayerCount = input.readByte();
	}

	void serialize (ByteStream& out) const override
	{
		out.addByte(_id);
		out.addString(_name);
		out.addString(_mapName);
		out.addInt(_port);
		out.addByte(_playerCount);
		out.addByte(_maxPlayerCount);
	}

	inline const std::string& getMapName () const
	{
		return _mapName;
	}

	inline const std::string& getName () const
	{
		return _name;
	}

	inline int getPort () const
	{
		return _port;
	}

	inline int getPlayerCount () const
	{
		return _playerCount;
	}

	inline int getMaxPlayerCount () const
	{
		return _maxPlayerCount;
	}
};
