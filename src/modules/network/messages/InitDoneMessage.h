#pragma once

#include "network/IProtocolMessage.h"

class InitDoneMessage: public IProtocolMessage {
private:
	uint16_t _playerId;
	uint8_t _packages;
	uint8_t _transfers;
	uint8_t _lives;
	uint16_t _hitpoints;
	bool _spectator;
	
public:
	InitDoneMessage (uint16_t playerId, uint8_t packages, uint8_t transfers, uint8_t lives, uint16_t hitpoints, bool spectator = false) :
			IProtocolMessage(protocol::PROTO_INITDONE),
			_playerId(playerId), _packages(packages), _transfers(transfers), _lives(lives), _hitpoints(hitpoints), _spectator(spectator)
	{
	}

	PROTOCOL_CLASS_FACTORY(InitDoneMessage);

	explicit InitDoneMessage (ByteStream& input) :
			IProtocolMessage(protocol::PROTO_INITDONE)
	{
		_playerId = input.readShort();
		_packages = input.readByte();
		_transfers = input.readByte();
		_lives = input.readByte();
		_hitpoints = input.readShort();
		_spectator = input.getSize() > 0 && input.readByte() != 0;
	}

	void serialize (ByteStream& out) const override
	{
		out.addByte(_id);
		out.addShort(_playerId);
		out.addByte(_packages);
		out.addByte(_transfers);
		out.addByte(_lives);
		out.addShort(_hitpoints);
		out.addByte(_spectator ? 1 : 0);
	}

	inline uint8_t getPackages () const
	{
		return _packages;
	}

	inline uint8_t getTransfers () const
	{
		return _transfers;
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

	inline bool isSpectator () const
	{
		return _spectator;
	}
};
