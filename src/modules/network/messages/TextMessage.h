#pragma once

#include "network/IProtocolMessage.h"

class TextMessage: public IProtocolMessage {
private:
	std::string _message;

public:
	explicit TextMessage (const std::string& message) :
			IProtocolMessage(protocol::PROTO_MESSAGE), _message(message)
	{
	}

	PROTOCOL_CLASS_FACTORY(TextMessage);

	explicit TextMessage (ByteStream& input) :
			IProtocolMessage(protocol::PROTO_MESSAGE)
	{
		_message = input.readString();
	}

	void serialize (ByteStream& out) const override
	{
		out.addByte(_id);
		out.addString(_message);
	}

	inline const std::string& getMessage () const
	{
		return _message;
	}
};
