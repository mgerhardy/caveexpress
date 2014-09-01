#pragma once

#include "shared/network/IProtocolMessage.h"
#include <string>

class ClientInitMessage: public IProtocolMessage {
private:
	std::string _name;
public:
	ClientInitMessage (const std::string& name) :
			IProtocolMessage(protocol::PROTO_CLIENTINIT), _name(name)
	{
	}

	ClientInitMessage (ByteStream& input) :
			IProtocolMessage(protocol::PROTO_CLIENTINIT)
	{
		_name = input.readString();
	}

	void serialize (ByteStream& out) const override
	{
		out.addByte(_id);
		out.addString(_name);
	}

	inline const std::string& getName () const
	{
		return _name;
	}
};
