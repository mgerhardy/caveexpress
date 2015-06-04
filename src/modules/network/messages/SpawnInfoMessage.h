#pragma once

#include "network/IProtocolMessage.h"
#include "common/EntityType.h"

class SpawnInfoMessage: public IProtocolMessage {
private:
	float _xpos;
	float _ypos;
	const EntityType* _entityType;
public:
	SpawnInfoMessage (float xpos, float ypos, const EntityType& entityType) :
			IProtocolMessage(protocol::PROTO_SPAWNINFO), _xpos(xpos), _ypos(ypos), _entityType(&entityType)
	{
	}

	PROTOCOL_CLASS_FACTORY(SpawnInfoMessage);

	explicit SpawnInfoMessage (ByteStream& input) :
			IProtocolMessage(protocol::PROTO_SPAWNINFO)
	{
		_xpos = input.readShortScaled();
		_ypos = input.readShortScaled();
		_entityType = &EntityType::get(input.readByte());
	}

	void serialize (ByteStream& out) const override
	{
		out.addByte(_id);
		out.addShortScaled(_xpos);
		out.addShortScaled(_ypos);
		out.addByte(_entityType->id);
	}

	inline float getX () const
	{
		return _xpos;
	}

	inline float getY () const
	{
		return _ypos;
	}

	inline const EntityType& getEntityType () const
	{
		return *_entityType;
	}
};
