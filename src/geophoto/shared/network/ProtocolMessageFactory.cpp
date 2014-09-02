#include "engine/common/network/ProtocolMessageFactory.h"
#include "engine/common/Logger.h"
#include "cavepacker/shared/network/ProtocolMessageTypes.h"
#include "cavepacker/shared/network/messages/ProtocolMessages.h"

ProtocolMessageFactory::ProtocolMessageFactory ()
{
}

IProtocolMessage *ProtocolMessageFactory::create (ByteStream& stream)
{
	const protocolId type = stream.readByte();
	debug(LOG_GENERAL, String::format("msg type => %i", (int)type));
	IProtocolMessage *msg = getForProtocolId(stream, type);
	if (msg == nullptr) {
		error(LOG_NET, String::format("unknown module type given: %i", type));
		stream.addByte(type, true);
		return nullptr;
	}
	return msg;
}

