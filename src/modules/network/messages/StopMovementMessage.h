#pragma once

#include "network/IProtocolMessage.h"
#include "common/Direction.h"
#include <string>

class StopMovementMessage: public IProtocolMessage {
private:
	Direction _direction;
	uint8_t _uid;

public:
	StopMovementMessage (Direction direction, uint8_t uid) :
			IProtocolMessage(protocol::PROTO_STOPMOVEMENT), _direction(direction), _uid(uid)
	{
	}

	PROTOCOL_CLASS_FACTORY(StopMovementMessage);

	explicit StopMovementMessage (ByteStream& input) :
			IProtocolMessage(protocol::PROTO_STOPMOVEMENT)
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
