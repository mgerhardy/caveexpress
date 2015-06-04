#pragma once

#include "network/IProtocolMessage.h"

class TargetCaveMessage: public IProtocolMessage {
private:
	uint8_t _targetCave;

public:
	TargetCaveMessage(uint8_t targetCave) :
			IProtocolMessage(protocol::PROTO_TARGETCAVE), _targetCave(targetCave) {
	}

	TargetCaveMessage(ByteStream& input) :
			IProtocolMessage(protocol::PROTO_TARGETCAVE) {
		_targetCave = input.readByte();
	}

	void serialize(ByteStream& out) const override
	{
		out.addByte(_id);
		out.addByte(_targetCave);
	}

	inline uint8_t getCaveNumber() const {
		return _targetCave;
	}
};
