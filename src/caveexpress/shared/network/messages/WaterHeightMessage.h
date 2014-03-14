#pragma once

#include "engine/common/network/IProtocolMessage.h"

class WaterHeightMessage: public IProtocolMessage {
private:
	float _height;
public:
	WaterHeightMessage (float height) :
			IProtocolMessage(protocol::PROTO_WATERHEIGHT), _height(height)
	{
	}

	WaterHeightMessage (ByteStream& input) :
			IProtocolMessage(protocol::PROTO_WATERHEIGHT)
	{
		_height = input.readShortScaled();
	}

	void serialize (ByteStream& out) const override
	{
		out.addByte(_id);
		out.addShortScaled(_height);
	}

	inline float getHeight () const
	{
		return _height;
	}
};
