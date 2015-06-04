#pragma once

#include "network/IProtocolMessage.h"
#include "common/Direction.h"
#include <string>

class StopMovementMessage: public IProtocolMessage {
private:
	Direction _direction;

public:
	explicit StopMovementMessage (Direction direction) :
			IProtocolMessage(protocol::PROTO_STOPMOVEMENT), _direction(direction)
	{
	}

	PROTOCOL_CLASS_FACTORY(StopMovementMessage);

	explicit StopMovementMessage (ByteStream& input) :
			IProtocolMessage(protocol::PROTO_STOPMOVEMENT)
	{
		_direction = input.readByte();
	}

	void serialize (ByteStream& out) const override
	{
		out.addByte(_id);
		out.addByte(_direction);
	}

	inline Direction getDirection () const
	{
		return _direction;
	}
};
