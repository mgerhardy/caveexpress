#pragma once

#include "network/IProtocolMessage.h"

namespace caveexpress {

class TargetCaveMessage: public IProtocolMessage {
private:
	uint8_t _targetCave;  // 0 none,  100 stone
	float _angle;  // arrow angle to target cave  -1 if none

public:
	TargetCaveMessage(uint8_t targetCave, float angle) :
			IProtocolMessage(protocol::PROTO_TARGETCAVE), _targetCave(targetCave), _angle(angle) {
	}

	PROTOCOL_CLASS_FACTORY(TargetCaveMessage);
	explicit TargetCaveMessage(ByteStream& input) :
			IProtocolMessage(protocol::PROTO_TARGETCAVE)
	{
		_targetCave = input.readByte();
		_angle = input.readFloat();
	}

	void serialize(ByteStream& out) const override
	{
		out.addByte(_id);
		out.addByte(_targetCave);
		out.addFloat(_angle);
	}

	inline uint8_t getCaveNumber() const {
		return _targetCave;
	}

	inline float getAngle() const {
		return _angle;
	}
};

}
