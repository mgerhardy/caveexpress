#pragma once

#include "network/IProtocolMessage.h"
#include <string>

class TimeRemainingMessage: public IProtocolMessage {
private:
	uint16_t _secondsRemaining;
public:
	TimeRemainingMessage (uint16_t secondsRemaining) :
			IProtocolMessage(protocol::PROTO_TIMEREMAINING), _secondsRemaining(secondsRemaining)
	{
	}

	PROTOCOL_CLASS_FACTORY(TimeRemainingMessage);

	TimeRemainingMessage (ByteStream& input) :
			IProtocolMessage(protocol::PROTO_TIMEREMAINING)
	{
		_secondsRemaining = input.readShort();
	}

	void serialize (ByteStream& out) const override
	{
		out.addByte(_id);
		out.addShort(_secondsRemaining);
	}

	inline uint16_t getSecondsRemaining () const
	{
		return _secondsRemaining;
	}
};
