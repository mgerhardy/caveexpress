#pragma once

#include "common/network/IProtocolMessage.h"

class FingerMovementMessage: public IProtocolMessage {
private:
	int _dx;
	int _dy;

public:
	FingerMovementMessage (int dx, int dy) :
			IProtocolMessage(protocol::PROTO_FINGERMOVEMENT), _dx(dx), _dy(dy)
	{
	}

	FingerMovementMessage (ByteStream& input) :
			IProtocolMessage(protocol::PROTO_FINGERMOVEMENT)
	{
		_dx = input.readShort();
		_dy = input.readShort();
	}

	void serialize (ByteStream& out) const override
	{
		out.addByte(_id);
		out.addShort(_dx);
		out.addShort(_dy);
	}

	inline int getDeltaX () const
	{
		return _dx;
	}

	inline int getDeltaY () const
	{
		return _dy;
	}
};
