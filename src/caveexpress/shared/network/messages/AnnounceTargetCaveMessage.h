#pragma once

#include "network/IProtocolMessage.h"

class AnnounceTargetCaveMessage: public IProtocolMessage {
private:
	uint16_t _npcId;
	uint16_t _delay;
	uint8_t _targetCave;

public:
	AnnounceTargetCaveMessage(uint16_t npcId, uint16_t delay, uint8_t targetCave) :
			IProtocolMessage(protocol::PROTO_ANNOUNCETARGETCAVE), _npcId(npcId), _delay(delay), _targetCave(targetCave) {
	}

	PROTOCOL_CLASS_FACTORY(AnnounceTargetCaveMessage);
	explicit AnnounceTargetCaveMessage(ByteStream& input) :
			IProtocolMessage(protocol::PROTO_ANNOUNCETARGETCAVE) {
		_npcId = input.readShort();
		_delay = input.readShort();
		_targetCave = input.readByte();
	}

	void serialize(ByteStream& out) const override
	{
		out.addByte(_id);
		out.addShort(_npcId);
		out.addShort(_delay);
		out.addByte(_targetCave);
	}

	inline uint8_t getCaveNumber() const {
		return _targetCave;
	}

	inline uint16_t getNpcId() const {
		return _npcId;
	}

	inline uint16_t getDelay() const {
		return _delay;
	}
};
