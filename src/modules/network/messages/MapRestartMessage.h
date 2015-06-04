#pragma once

#include "network/IProtocolMessage.h"

class MapRestartMessage: public IProtocolMessage {
private:
	uint16_t _delay;
public:
	MapRestartMessage (uint16_t delay) :
			IProtocolMessage(protocol::PROTO_MAPRESTART), _delay(delay)
	{
	}

	PROTOCOL_CLASS_FACTORY(MapRestartMessage);

	MapRestartMessage (ByteStream& input) :
			IProtocolMessage(protocol::PROTO_MAPRESTART)
	{
		_delay = input.readShort();
	}

	void serialize (ByteStream& out) const override
	{
		out.addByte(_id);
		out.addShort(_delay);
	}

	inline uint16_t getDelay () const
	{
		return _delay;
	}
};
