#pragma once

#include "network/IProtocolMessage.h"

class RemoveEntityMessage: public IProtocolMessage {
private:
	uint16_t _entityId;
	bool _fadeOut;
public:
	RemoveEntityMessage (uint16_t entityId, bool fadeOut) :
			IProtocolMessage(protocol::PROTO_REMOVEENTITY), _entityId(entityId), _fadeOut(fadeOut)
	{
	}

	RemoveEntityMessage (ByteStream& input) :
			IProtocolMessage(protocol::PROTO_REMOVEENTITY)
	{
		_entityId = input.readShort();
		_fadeOut = input.readBool();
	}

	void serialize (ByteStream& out) const override
	{
		out.addByte(_id);
		out.addShort(_entityId);
		out.addBool(_fadeOut);
	}

	inline uint16_t getEntityId () const
	{
		return _entityId;
	}

	inline bool isFadeOut () const
	{
		return _fadeOut;
	}
};
