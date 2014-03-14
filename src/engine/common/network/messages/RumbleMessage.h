#pragma once

#include "engine/common/network/IProtocolMessage.h"

class RumbleMessage: public IProtocolMessage {
private:
	float _strength;
	int _lengthMillis;

public:
	RumbleMessage (float strength, int lengthMillis) :
			IProtocolMessage(protocol::PROTO_RUMBLE), _strength(strength), _lengthMillis(lengthMillis)
	{
	}

	RumbleMessage (ByteStream& input) :
			IProtocolMessage(protocol::PROTO_RUMBLE)
	{
		_strength = input.readFloat();
		_lengthMillis = input.readInt();
	}

	void serialize (ByteStream& out) const override
	{
		out.addByte(_id);
		out.addFloat(_strength);
		out.addInt(_lengthMillis);
	}

	inline float getStrength () const
	{
		return _strength;
	}

	inline int getLengthMillis () const
	{
		return _lengthMillis;
	}
};
