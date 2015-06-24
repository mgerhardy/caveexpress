#pragma once

#include "caveexpress/server/entities/IEntity.h"
#include "caveexpress/server/entities/Platform.h"
#include "caveexpress/server/entities/npcs/NPCPackage.h"
#include "common/Direction.h"
#include "caveexpress/shared/constants/PlayerState.h"
#include "common/SoundType.h"
#include "common/ConfigVar.h"
#include "common/Log.h"
#include <memory>
#include "network/IProtocolHandler.h"

namespace caveexpress {

// forward decl
class Map;
class NPCFriendly;
class CaveMapTile;
class CollectableEntity;

#define MAX_COLLECTED 4

typedef enum
{
	CRASH_NONE,
	CRASH_NPC_WALKING,
	CRASH_NPC_MAMMUT,
	CRASH_NPC_FISH,
	CRASH_NPC_FLYING,
	CRASH_DAMAGE,
	CRASH_MAP_FAILED
} PlayerCrashReason;

class Player: public IEntity {
private:
	// the entity the player is current touching
	Platform* _touching;

	uint16_t _hitpoints;
	uint8_t _lives;

	NPCFriendly* _collectedNPC;

	b2Vec2 _acceleration;

	bool _fingerAcceleration;
	int _accelerateX;
	int _accelerateY;

	ClientId _clientId;

	struct Collected {
		const EntityType *entityType;
		// beware - this might not always be a valid pointer - use the entity type
		// pointer to decide whether you have a valid pointer or not
		CollectableEntity *entity;
	};

	Collected _collectedEntities[MAX_COLLECTED];

	uint32_t _lastAccelerate;

	std::string _name;

	ConfigVarPtr _godMode;

	uint32_t _lastFruitCollected;
	int8_t _fruitsCollectedInARow;

	b2RevoluteJoint *_revoluteJoint;

	PlayerCrashReason _crashReason;

	float getCompleteMass () const;

public:
	Player (Map& map, ClientId clientId);
	virtual ~Player ();

	void resetAcceleration (Direction dir);
	void accelerate (Direction dir);
	void setAcceleration (int dx, int dy);
	void resetAcceleration ();
	void applyForce (const b2Vec2& v);

	// sets the landing spot the player is currently landed on
	void setPlatform (Platform* entity);
	bool isLanded () const;
	bool isCloseOverSolid (float distance = 1.0f) const;
	bool isLandedOn (const CaveMapTile *cave) const;
	void setCrashed (const PlayerCrashReason& reason);
	bool isCrashed () const;
	const PlayerCrashReason& getCrashReason () const;
	// returns true if the player does not carry anything
	bool isFree () const;
	bool isTransfering(NPCFriendly* npc) const;
	bool canCarry (const IEntity* entity) const;

	uint16_t getHitpoints () const;
	uint8_t getLives () const;
	void setLives (uint8_t lives);
	void reduceLive ();
	void addLife ();
	bool isDead () const;

	void onDeath ();

	bool collect (CollectableEntity* entity);
	void drop ();

	void subtractHitpoints (uint16_t hitpoints);
	void addHitpoints (uint16_t hitpoints);

	void createBody (const b2Vec2 &pos);

	void setCollectedNPC(NPCFriendly *npc);
	void reset ();

	ClientId getClientId () const;
	const std::string& getName () const;
	void setName (const std::string& name);

	// IEntity
	void print (std::ostream &stream, int level) const override;
	bool shouldApplyWind () const override;
	void update (uint32_t deltaTime) override;
	void onPreSolve (b2Contact* contact, IEntity* entity, const b2Manifold* oldManifold) override;
	bool shouldCollide (const IEntity* entity) const override;
};

inline ClientId Player::getClientId () const
{
	return _clientId;
}

inline const std::string& Player::getName () const
{
	return _name;
}

inline void Player::setName (const std::string& name)
{
	_name = name;
}

inline void Player::reset ()
{
	setState(PlayerState::PLAYER_FLYING);
	_acceleration = b2Vec2_zero;
	_touching = nullptr;
}

inline bool Player::isCrashed () const
{
	return getState() == PlayerState::PLAYER_CRASHED;
}

inline uint16_t Player::getHitpoints () const
{
	return _hitpoints;
}

inline uint8_t Player::getLives () const
{
	return _lives;
}

inline void Player::setLives (uint8_t lives)
{
	_lives = lives;
}

inline void Player::reduceLive ()
{
	--_lives;
}

inline void Player::addLife ()
{
	++_lives;
}

inline bool Player::isDead () const
{
	return _lives <= 0;
}

inline bool Player::isFree () const
{
	if (_collectedNPC != nullptr)
		return false;
	for (int i = 0; i < MAX_COLLECTED; ++i) {
		const EntityType *entityType = _collectedEntities[i].entityType;
		if (entityType != nullptr)
			return false;
	}
	return true;
}

inline bool Player::isTransfering(NPCFriendly *npc) const {
	return _collectedNPC == npc;
}

inline bool Player::isLandedOn (const CaveMapTile *cave) const
{
	if (_touching == nullptr || !isLanded())
		return false;
	return _touching->getCave() == cave;
}

inline const PlayerCrashReason& Player::getCrashReason () const
{
	return _crashReason;
}

}
