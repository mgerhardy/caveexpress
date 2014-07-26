#include "engine/common/network/ProtocolMessageFactory.h"
#include "engine/common/Logger.h"

ProtocolMessageFactory::ProtocolMessageFactory ()
{
}

IProtocolMessage *ProtocolMessageFactory::create (ByteStream& stream)
{
	const protocolId type = stream.readByte();
	IProtocolMessage *msg = getForProtocolId(stream, type);
	if (msg == nullptr) {
		error(LOG_NET, String::format("unknown module type given: %i", type));
		stream.addByte(type, true);
	}
	return msg;
}
