#pragma once

#include "network/IProtocolMessage.h"
#include <string>

class AddRopeMessage: public IProtocolMessage {
private:
	uint16_t _entityId1;
	uint16_t _entityId2;
public:
	AddRopeMessage (uint16_t entityId1, uint16_t entityId2) :
			IProtocolMessage(protocol::PROTO_ADDROPE), _entityId1(entityId1), _entityId2(entityId2)
	{
	}

	AddRopeMessage (ByteStream& input) :
			IProtocolMessage(protocol::PROTO_ADDROPE)
	{
		_entityId1 = input.readShort();
		_entityId2 = input.readShort();
	}

	void serialize (ByteStream& out) const override
	{
		out.addByte(_id);
		out.addShort(_entityId1);
		out.addShort(_entityId2);
	}

	inline uint16_t getEntityId1 () const
	{
		return _entityId1;
	}

	inline uint16_t getEntityId2 () const
	{
		return _entityId2;
	}
};
