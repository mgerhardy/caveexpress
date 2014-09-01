#pragma once

#include "shared/network/IProtocolMessage.h"
#include <string>

class PingMessage: public IProtocolMessage {
private:
	std::string _name;
	int _port;
	int _maxPlayerCount;

public:
	PingMessage (const std::string& name, int port, int maxPlayerCount) :
			IProtocolMessage(protocol::PROTO_PING), _name(name), _port(port), _maxPlayerCount(
					maxPlayerCount)
	{
	}

	PingMessage (ByteStream& input) :
			IProtocolMessage(protocol::PROTO_PING)
	{
		_name = input.readString();
		_port = input.readInt();
		_maxPlayerCount = input.readByte();
	}

	void serialize (ByteStream& out) const override
	{
		out.addByte(_id);
		out.addString(_name);
		out.addInt(_port);
		out.addByte(_maxPlayerCount);
	}

	inline const std::string& getName () const
	{
		return _name;
	}

	inline int getPort () const
	{
		return _port;
	}

	inline int getMaxPlayerCount () const
	{
		return _maxPlayerCount;
	}
};
