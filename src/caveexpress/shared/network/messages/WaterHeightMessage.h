#pragma once

#include "network/IProtocolMessage.h"

namespace caveexpress {

class WaterHeightMessage: public IProtocolMessage {
private:
	float _height;
public:
	WaterHeightMessage (float height) :
			IProtocolMessage(protocol::PROTO_WATERHEIGHT), _height(height)
	{
	}

	PROTOCOL_CLASS_FACTORY(WaterHeightMessage);
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

}
