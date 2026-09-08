#pragma once

#include "network/IProtocolMessage.h"
#include "caveexpress/shared/network/ProtocolMessageTypes.h"

namespace caveexpress {

/**
 * Per-player HUD snapshot for spectators (hitpoints, lives, carried item, target cave).
 * Broadcast to every client when that state changes.
 */
class PlayerHudMessage: public IProtocolMessage {
private:
	uint16_t _entityId;
	uint16_t _hitpoints;
	uint8_t _lives;
	uint8_t _targetCave;
	uint8_t _collectedTypeId;
public:
	PlayerHudMessage (uint16_t entityId, uint16_t hitpoints, uint8_t lives, uint8_t targetCave, uint8_t collectedTypeId) :
			IProtocolMessage(protocol::PROTO_PLAYERHUD), _entityId(entityId), _hitpoints(hitpoints), _lives(lives),
			_targetCave(targetCave), _collectedTypeId(collectedTypeId)
	{
	}

	PROTOCOL_CLASS_FACTORY(PlayerHudMessage);
	explicit PlayerHudMessage (ByteStream& input) :
			IProtocolMessage(protocol::PROTO_PLAYERHUD)
	{
		_entityId = input.readShort();
		_hitpoints = input.readShort();
		_lives = input.readByte();
		_targetCave = input.readByte();
		_collectedTypeId = input.readByte();
	}

	void serialize (ByteStream& out) const override
	{
		out.addByte(_id);
		out.addShort(_entityId);
		out.addShort(_hitpoints);
		out.addByte(_lives);
		out.addByte(_targetCave);
		out.addByte(_collectedTypeId);
	}

	inline uint16_t getEntityId () const
	{
		return _entityId;
	}

	inline uint16_t getHitpoints () const
	{
		return _hitpoints;
	}

	inline uint8_t getLives () const
	{
		return _lives;
	}

	inline uint8_t getTargetCave () const
	{
		return _targetCave;
	}

	inline uint8_t getCollectedTypeId () const
	{
		return _collectedTypeId;
	}
};

}
