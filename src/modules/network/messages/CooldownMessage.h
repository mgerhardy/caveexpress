#pragma once

#include "network/IProtocolMessage.h"
#include "common/Cooldown.h"

class CooldownMessage: public IProtocolMessage {
private:
	const Cooldown* _cooldown;
public:
	CooldownMessage (const Cooldown& cooldown) :
			IProtocolMessage(protocol::PROTO_COOLDOWN), _cooldown(&cooldown)
	{
	}

	PROTOCOL_CLASS_FACTORY(CooldownMessage);

	explicit CooldownMessage (ByteStream& input) :
			IProtocolMessage(protocol::PROTO_COOLDOWN)
	{
		_cooldown = &Cooldown::get(input.readByte());
	}

	void serialize (ByteStream& out) const override
	{
		out.addByte(_id);
		out.addByte(_cooldown->id);
	}

	inline const Cooldown& getCooldown () const
	{
		return *_cooldown;
	}
};
