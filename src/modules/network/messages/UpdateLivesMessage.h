#pragma once

#include "network/IProtocolMessage.h"

class UpdateLivesMessage: public IProtocolMessage {
private:
	uint8_t _lives;

public:
	UpdateLivesMessage (uint8_t lives) :
			IProtocolMessage(protocol::PROTO_UPDATELIVES), _lives(lives)
	{
	}

	PROTOCOL_CLASS_FACTORY(UpdateLivesMessage);

	UpdateLivesMessage (ByteStream& input) :
			IProtocolMessage(protocol::PROTO_UPDATELIVES)
	{
		_lives = input.readByte();
	}

	void serialize (ByteStream& out) const override
	{
		out.addByte(_id);
		out.addByte(_lives);
	}

	inline uint8_t getLives () const
	{
		return _lives;
	}
};
