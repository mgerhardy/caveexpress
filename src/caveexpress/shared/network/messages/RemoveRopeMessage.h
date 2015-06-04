#pragma once

#include "network/IProtocolMessage.h"
#include <string>

class RemoveRopeMessage: public IProtocolMessage {
private:
	uint16_t _entityId;
public:
	RemoveRopeMessage (uint16_t entityId) :
			IProtocolMessage(protocol::PROTO_REMOVEROPE), _entityId(entityId)
	{
	}

	PROTOCOL_CLASS_FACTORY(RemoveRopeMessage);

	RemoveRopeMessage (ByteStream& input) :
			IProtocolMessage(protocol::PROTO_REMOVEROPE)
	{
		_entityId = input.readShort();
	}

	void serialize (ByteStream& out) const override
	{
		out.addByte(_id);
		out.addShort(_entityId);
	}

	inline uint16_t getEntityId () const
	{
		return _entityId;
	}
};
