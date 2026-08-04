#pragma once

#include "network/IProtocolMessage.h"
#include "caveexpress/shared/network/ProtocolMessageTypes.h"

namespace caveexpress {

/**
 * @brief Replicates how far a gate protrudes from the wall (0 = open/recessed, 255 = fully closed).
 */
class GateStateMessage: public IProtocolMessage {
private:
	uint16_t _entityId;
	uint8_t _openAmount;
public:
	GateStateMessage (uint16_t entityId, uint8_t openAmount) :
			IProtocolMessage(protocol::PROTO_GATESTATE), _entityId(entityId), _openAmount(openAmount)
	{
	}

	PROTOCOL_CLASS_FACTORY(GateStateMessage);
	GateStateMessage (ByteStream& input) :
			IProtocolMessage(protocol::PROTO_GATESTATE)
	{
		_entityId = input.readShort();
		_openAmount = input.readByte();
	}

	void serialize (ByteStream& out) const override
	{
		out.addByte(_id);
		out.addShort(_entityId);
		out.addByte(_openAmount);
	}

	inline uint16_t getEntityId () const
	{
		return _entityId;
	}

	inline uint8_t getOpenAmount () const
	{
		return _openAmount;
	}
};

}
