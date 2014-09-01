#pragma once

#include "shared/network/IProtocolMessage.h"
#include "shared/constants/SoundType.h"

class SoundMessage: public IProtocolMessage {
private:
	float _x;
	float _y;
	SoundType _sound;
public:
	SoundMessage (float x, float y, SoundType sound) :
			IProtocolMessage(protocol::PROTO_SOUND), _x(x), _y(y), _sound(sound)
	{
	}

	SoundMessage (ByteStream& input) :
			IProtocolMessage(protocol::PROTO_SOUND)
	{
		_x = input.readShortScaled();
		_y = input.readShortScaled();
		_sound = static_cast<SoundType>(input.readByte());
	}

	void serialize (ByteStream& out) const override
	{
		out.addByte(_id);
		out.addShortScaled(_x);
		out.addShortScaled(_y);
		out.addByte(_sound);
	}

	inline float getX () const
	{
		return _x;
	}

	inline float getY () const
	{
		return _y;
	}

	inline SoundType getSoundType () const
	{
		return _sound;
	}
};
