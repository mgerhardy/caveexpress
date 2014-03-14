#pragma once

#include "engine/common/network/IProtocolMessage.h"
#include "engine/common/SoundType.h"

class SoundMessage: public IProtocolMessage {
private:
	float _x;
	float _y;
	const SoundType* _sound;
public:
	SoundMessage (float x, float y, const SoundType& sound) :
			IProtocolMessage(protocol::PROTO_SOUND), _x(x), _y(y), _sound(&sound)
	{
	}

	SoundMessage (ByteStream& input) :
			IProtocolMessage(protocol::PROTO_SOUND)
	{
		_x = input.readShortScaled();
		_y = input.readShortScaled();
		_sound = &SoundType::get(input.readByte());
	}

	void serialize (ByteStream& out) const override
	{
		out.addByte(_id);
		out.addShortScaled(_x);
		out.addShortScaled(_y);
		out.addByte(_sound->id);
	}

	inline float getX () const
	{
		return _x;
	}

	inline float getY () const
	{
		return _y;
	}

	inline const SoundType& getSoundType () const
	{
		return *_sound;
	}
};
