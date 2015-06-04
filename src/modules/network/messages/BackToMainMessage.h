#pragma once

#include "network/IProtocolMessage.h"
#include <string>

class BackToMainMessage: public IProtocolMessage {
private:
	std::string _window;

public:
	explicit BackToMainMessage (const std::string& window) :
			IProtocolMessage(protocol::PROTO_BACKTOMAIN), _window(window)
	{
	}

	PROTOCOL_CLASS_FACTORY(BackToMainMessage);

	explicit BackToMainMessage (ByteStream& input) :
			IProtocolMessage(protocol::PROTO_BACKTOMAIN)
	{
		_window = input.readString();
	}

	void serialize (ByteStream& out) const override
	{
		out.addByte(_id);
		out.addString(_window);
	}

	inline const std::string& getWindow () const
	{
		return _window;
	}
};
