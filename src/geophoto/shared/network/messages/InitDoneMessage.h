#pragma once

#include "shared/network/IProtocolMessage.h"

class InitDoneMessage: public IProtocolMessage {
private:
	uint16_t _playerId;

public:
	InitDoneMessage (uint16_t playerId) :
			IProtocolMessage(protocol::PROTO_INITDONE), _playerId(playerId)
	{
	}

	InitDoneMessage (ByteStream& input) :
			IProtocolMessage(protocol::PROTO_INITDONE)
	{
		_playerId = input.readShort();
	}

	void serialize (ByteStream& out) const override
	{
		out.addByte(_id);
		out.addShort(_playerId);
	}

	inline uint16_t getPlayerId () const
	{
		return _playerId;
	}
};
