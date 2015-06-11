#pragma once

#include "network/IProtocolMessage.h"
#include "caveexpress/shared/network/ProtocolMessageTypes.h"
#include <string>

class AddCaveMessage: public IProtocolMessage {
private:
	uint16_t _entityId;
	uint8_t _number;
	bool _state;
public:
	AddCaveMessage (uint16_t entityId, uint8_t number, bool state) :
			IProtocolMessage(protocol::PROTO_ADDCAVE), _entityId(entityId), _number(number), _state(state)
	{
	}

	PROTOCOL_CLASS_FACTORY(AddCaveMessage);
	AddCaveMessage (ByteStream& input) :
			IProtocolMessage(protocol::PROTO_ADDCAVE)
	{
		_entityId = input.readShort();
		_number = input.readByte();
		_state = input.readBool();
	}

	void serialize (ByteStream& out) const override
	{
		out.addByte(_id);
		out.addShort(_entityId);
		out.addByte(_number);
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

	inline uint8_t getCaveNumber () const
	{
		return _number;
	}
};
