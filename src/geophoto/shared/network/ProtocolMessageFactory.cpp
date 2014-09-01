#include "ProtocolMessageFactory.h"
#include "ProtocolMessageTypes.h"
#include "IProtocolMessage.h"
#include "shared/ByteStream.h"
#include "shared/Logger.h"

#include "messages/PingMessage.h"
#include "messages/ClientInitMessage.h"
#include "messages/InitDoneMessage.h"
#include "messages/SoundMessage.h"
#include "messages/ProtocolMessages.h"

ProtocolMessageFactory::ProtocolMessageFactory ()
{
}

IProtocolMessage *ProtocolMessageFactory::create (ByteStream& stream)
{
	const protocolId type = stream.readByte();

	switch (type) {
	case protocol::PROTO_PING:
		return new PingMessage(stream);
	case protocol::PROTO_CLIENTINIT:
		return new ClientInitMessage(stream);
	case protocol::PROTO_DISCONNECT:
		return new DisconnectMessage(stream);
	case protocol::PROTO_INITDONE:
		return new InitDoneMessage(stream);
	case protocol::PROTO_SOUND:
		return new SoundMessage(stream);
	default:
		stream.addByte(type, true);
		error(LOG_NETWORK, String::format("unknown module type given: %i", type));
		return nullptr;
	}
}
