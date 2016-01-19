#pragma once

#include "network/IProtocolMessage.h"
#include "common/Direction.h"
#include <string>

class MovementMessage: public IProtocolMessage {
private:
	Direction _direction;
	uint8_t _uid;

public:
	MovementMessage (Direction direction, uint8_t uid) :
			IProtocolMessage(protocol::PROTO_MOVEMENT), _direction(direction), _uid(uid)
	{
	}

	PROTOCOL_CLASS_FACTORY(MovementMessage);

	explicit MovementMessage (ByteStream& input) :
			IProtocolMessage(protocol::PROTO_MOVEMENT)
	{
		_direction = input.readByte();
		_uid = input.readByte();
	}

	void serialize (ByteStream& out) const override
	{
		out.addByte(_id);
		out.addByte(_direction);
		out.addByte(_uid);
	}

	inline uint8_t getUID () const
	{
		return _uid;
	}

	inline Direction getDirection () const
	{
		return _direction;
	}
};
