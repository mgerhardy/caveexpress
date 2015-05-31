#pragma once

#include "network/IProtocolMessage.h"
#include "common/SpriteDefinition.h"

class UpdateEntityMessage: public IProtocolMessage {
private:
	uint16_t _entityId;
	float _x;
	float _y;
	EntityAngle _angle;
	uint8_t _state;

public:
	UpdateEntityMessage (uint16_t entityId, float x, float y, EntityAngle angle, uint8_t state) :
			IProtocolMessage(protocol::PROTO_UPDATEENTITY), _entityId(entityId), _x(x), _y(y), _angle(angle), _state(state)
	{
	}

	UpdateEntityMessage (ByteStream& input) :
			IProtocolMessage(protocol::PROTO_UPDATEENTITY)
	{
		_entityId = input.readShort();
		_x = input.readShortScaled();
		_y = input.readShortScaled();
		_angle = static_cast<EntityAngle>(input.readShort());
		_state = input.readByte();
	}

	void serialize (ByteStream& out) const override
	{
		out.addByte(_id);
		out.addShort(_entityId);
		out.addShortScaled(_x);
		out.addShortScaled(_y);
		out.addShort(_angle);
		out.addByte(_state);
	}

	inline uint16_t getEntityId () const
	{
		return _entityId;
	}

	inline float getX () const
	{
		return _x;
	}

	inline float getY () const
	{
		return _y;
	}

	inline EntityAngle getAngle () const
	{
		return _angle;
	}

	inline uint8_t getState () const
	{
		return _state;
	}
};
