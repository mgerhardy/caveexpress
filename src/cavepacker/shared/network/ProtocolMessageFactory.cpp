#include "network/ProtocolMessageFactory.h"
#include "common/Logger.h"
#include "cavepacker/shared/network/ProtocolMessageTypes.h"
#include "cavepacker/shared/network/messages/ProtocolMessages.h"

ProtocolMessageFactory::ProtocolMessageFactory ()
{
}

IProtocolMessage *ProtocolMessageFactory::create (ByteStream& stream) const
{
	const protocolId type = stream.readByte();
	debug(LOG_GENERAL, String::format("msg type => %i", (int)type));
	IProtocolMessage *msg = getForProtocolId(stream, type);
	if (msg != nullptr)
		return msg;
	switch (type) {
	case protocol::PROTO_AUTOSOLVE:
		return new AutoSolveStartedMessage();
	case protocol::PROTO_AUTOSOLVEABORT:
		return new AutoSolveAbortedMessage();
	case protocol::PROTO_UNDO:
		return new UndoMessage();
	default:
		error(LOG_NET, String::format("unknown module type given: %i", type));
		stream.addByte(type, true);
		break;
	}
	return msg;
}
