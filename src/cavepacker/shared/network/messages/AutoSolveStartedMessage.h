#pragma once

#include "engine/common/network/IProtocolMessage.h"
#include "cavepacker/shared/network/ProtocolMessageTypes.h"

class AutoSolveStartedMessage: public IProtocolMessage {
public:
	AutoSolveStartedMessage () :
			IProtocolMessage(protocol::PROTO_AUTOSOLVE)
	{
	}

	AutoSolveStartedMessage (ByteStream& input) :
			IProtocolMessage(protocol::PROTO_AUTOSOLVE)
	{
	}

	void serialize (ByteStream& out) const override
	{
		out.addByte(_id);
	}
};
