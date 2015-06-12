#pragma once

#include "network/IProtocolMessage.h"
#include <string>

namespace caveexpress {

class LightStateMessage: public IProtocolMessage {
private:
	uint16_t _entityId;
	bool _state;
public:
	LightStateMessage (uint16_t entityId, bool state) :
			IProtocolMessage(protocol::PROTO_LIGHTSTATE), _entityId(entityId), _state(state)
	{
	}

	PROTOCOL_CLASS_FACTORY(LightStateMessage);
	LightStateMessage (ByteStream& input) :
			IProtocolMessage(protocol::PROTO_LIGHTSTATE)
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

}
