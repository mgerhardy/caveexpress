#pragma once

#include "network/IProtocolMessage.h"
#include "common/Direction.h"
#include <string>

class MovementMessage: public IProtocolMessage {
private:
	Direction _direction;

public:
	MovementMessage (Direction direction) :
			IProtocolMessage(protocol::PROTO_MOVEMENT), _direction(direction)
	{
	}

	MovementMessage (ByteStream& input) :
			IProtocolMessage(protocol::PROTO_MOVEMENT)
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
