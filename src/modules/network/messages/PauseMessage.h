#pragma once

#include "network/IProtocolMessage.h"

class PauseMessage: public IProtocolMessage {
private:
	bool _pause;
public:
	explicit PauseMessage (bool pause) :
			IProtocolMessage(protocol::PROTO_PAUSE), _pause(pause)
	{
	}

	PROTOCOL_CLASS_FACTORY(PauseMessage);

	explicit PauseMessage (ByteStream& input) :
			IProtocolMessage(protocol::PROTO_PAUSE)
	{
		_pause = input.readBool();
	}

	void serialize (ByteStream& out) const override
	{
		out.addByte(_id);
		out.addBool(_pause);
	}

	inline bool isPause () const
	{
		return _pause;
	}
};
