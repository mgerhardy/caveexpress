#pragma once

#include "network/IProtocolMessage.h"

class TextMessage: public IProtocolMessage {
private:
	std::string _message;
	uint32_t _delayMillis;

public:
	explicit TextMessage (const std::string& message, uint32_t delayMillis = 3000) :
			IProtocolMessage(protocol::PROTO_MESSAGE), _message(message), _delayMillis(delayMillis)
	{
	}

	PROTOCOL_CLASS_FACTORY(TextMessage);

	explicit TextMessage (ByteStream& input) :
			IProtocolMessage(protocol::PROTO_MESSAGE), _delayMillis(3000)
	{
		_message = input.readString();
		if (!input.empty())
			_delayMillis = static_cast<uint32_t>(input.readInt());
	}

	void serialize (ByteStream& out) const override
	{
		out.addByte(_id);
		out.addString(_message);
		out.addInt(static_cast<int32_t>(_delayMillis));
	}

	inline const std::string& getMessage () const
	{
		return _message;
	}

	inline uint32_t getDelayMillis () const
	{
		return _delayMillis;
	}
};
