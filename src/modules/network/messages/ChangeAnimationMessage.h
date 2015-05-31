#pragma once

#include "network/IProtocolMessage.h"
#include "common/Animation.h"

class ChangeAnimationMessage: public IProtocolMessage {
private:
	uint16_t _entityId;
	const Animation* _animation;
public:
	ChangeAnimationMessage (uint16_t entityId, const Animation& animation) :
			IProtocolMessage(protocol::PROTO_CHANGEANIMATION), _entityId(entityId), _animation(&animation)
	{
	}

	ChangeAnimationMessage (ByteStream& input) :
			IProtocolMessage(protocol::PROTO_CHANGEANIMATION)
	{
		_entityId = input.readShort();
		_animation = &Animation::get(input.readByte());
	}

	void serialize (ByteStream& out) const override
	{
		out.addByte(_id);
		out.addShort(_entityId);
		out.addByte(_animation->id);
	}

	inline uint16_t getEntityId () const
	{
		return _entityId;
	}

	inline const Animation& getAnimation () const
	{
		return *_animation;
	}
};
