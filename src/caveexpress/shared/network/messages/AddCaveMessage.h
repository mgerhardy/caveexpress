#pragma once

#include "common/network/IProtocolMessage.h"
#include "caveexpress/shared/network/ProtocolMessageTypes.h"
#include <string>

class AddCaveMessage: public IProtocolMessage {
private:
	uint16_t _entityId;
	bool _state;
public:
	AddCaveMessage (uint16_t entityId, bool state) :
			IProtocolMessage(protocol::PROTO_ADDCAVE), _entityId(entityId), _state(state)
	{
	}

	AddCaveMessage (ByteStream& input) :
			IProtocolMessage(protocol::PROTO_ADDCAVE)
	{
		_entityId = input.readShort();
		_state = input.readBool();
	}

	void serialize (ByteStream& out) const override
	{
		out.addByte(_id);
		out.addShort(_entityId);
		out.addBool(_state);
	}

	inline uint16_t getEntityId () const
	{
		return _entityId;
	}

	inline bool getState () const
	{
		return _state;
	}
};
