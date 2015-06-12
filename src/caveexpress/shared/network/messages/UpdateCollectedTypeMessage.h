#pragma once

#include "network/IProtocolMessage.h"
#include "common/EntityType.h"

namespace caveexpress {

class UpdateCollectedTypeMessage: public IProtocolMessage {
private:
	const EntityType* _entityType;
	bool _collected;
public:
	UpdateCollectedTypeMessage (const EntityType& entityType, bool collected) :
			IProtocolMessage(protocol::PROTO_UPDATECOLLECTEDTYPE), _entityType(&entityType), _collected(collected)
	{
	}

	PROTOCOL_CLASS_FACTORY(UpdateCollectedTypeMessage);
	UpdateCollectedTypeMessage (ByteStream& input) :
			IProtocolMessage(protocol::PROTO_UPDATECOLLECTEDTYPE)
	{
		_entityType = &EntityType::get(input.readByte());
		_collected = input.readByte();
	}

	void serialize (ByteStream& out) const override
	{
		out.addByte(_id);
		out.addByte(_entityType->id);
		out.addByte(_collected);
	}

	inline const EntityType& getEntityType () const
	{
		return *_entityType;
	}

	inline bool isCollected () const
	{
		return _collected;
	}
};

}
