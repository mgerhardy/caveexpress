#include "engine/common/network/ProtocolMessageFactory.h"
#include "ProtocolMessageTypes.h"
#include "engine/common/Logger.h"

#include "messages/AddCaveMessage.h"
#include "messages/AddRopeMessage.h"
#include "messages/LightStateMessage.h"
#include "messages/ProtocolMessages.h"
#include "messages/RemoveRopeMessage.h"
#include "messages/UpdateCollectedTypeMessage.h"
#include "messages/WaterHeightMessage.h"
#include "messages/WaterImpactMessage.h"

ProtocolMessageFactory::ProtocolMessageFactory ()
{
}

IProtocolMessage *ProtocolMessageFactory::create (ByteStream& stream)
{
	const protocolId type = stream.readByte();
	//debug(LOG_GENERAL, String::format("msg type => %i", (int)type));
	IProtocolMessage *msg = getForProtocolId(stream, type);
	if (msg != nullptr)
		return msg;
	switch (type) {
	case protocol::PROTO_DROP:
		return new DropMessage();
	case protocol::PROTO_REMOVEROPE:
		return new RemoveRopeMessage(stream);
	case protocol::PROTO_WATERHEIGHT:
		return new WaterHeightMessage(stream);
	case protocol::PROTO_WATERIMPACT:
		return new WaterImpactMessage(stream);
	case protocol::PROTO_ADDCAVE:
		return new AddCaveMessage(stream);
	case protocol::PROTO_LIGHTSTATE:
		return new LightStateMessage(stream);
	case protocol::PROTO_UPDATECOLLECTEDTYPE:
		return new UpdateCollectedTypeMessage(stream);
	case protocol::PROTO_ADDROPE:
		return new AddRopeMessage(stream);
	default:
		stream.addByte(type, true);
		error(LOG_NET, String::format("unknown module type given: %i", type));
		return nullptr;
	}
}
