#pragma once

#include "network/IProtocolMessage.h"

class InitDoneMessage: public IProtocolMessage {
private:
	uint16_t _playerId;
	uint8_t _packages;
	uint8_t _lives;
	uint16_t _hitpoints;

public:
	InitDoneMessage (uint16_t playerId, uint8_t packages, uint8_t lives, uint16_t hitpoints) :
			IProtocolMessage(protocol::PROTO_INITDONE), _playerId(playerId), _packages(packages), _lives(lives), _hitpoints(hitpoints)
	{
	}

	PROTOCOL_CLASS_FACTORY(InitDoneMessage);

	InitDoneMessage (ByteStream& input) :
			IProtocolMessage(protocol::PROTO_INITDONE)
	{
		_playerId = input.readShort();
		_packages = input.readByte();
		_lives = input.readByte();
		_hitpoints = input.readShort();
	}

	void serialize (ByteStream& out) const override
	{
		out.addByte(_id);
		out.addShort(_playerId);
		out.addByte(_packages);
		out.addByte(_lives);
		out.addShort(_hitpoints);
	}

	inline uint8_t getPackages () const
	{
		return _packages;
	}

	inline uint16_t getPlayerId () const
	{
		return _playerId;
	}

	inline uint8_t getLives () const
	{
		return _lives;
	}

	inline uint16_t getHitpoints () const
	{
		return _hitpoints;
	}
};
