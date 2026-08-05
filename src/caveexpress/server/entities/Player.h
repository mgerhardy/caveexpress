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
class Package;

#define MAX_COLLECTED 4

typedef enum
{
	CRASH_NONE,
	CRASH_NPC_WALKING,
	CRASH_NPC_MAMMUT,
	CRASH_NPC_FISH,
	CRASH_NPC_FLYING,
	CRASH_DAMAGE,
	CRASH_LAVA,
	CRASH_MAP_FAILED
} PlayerCrashReason;

/**
 * @brief The player entity that can either transfer friendly npcs or carry
 * packages to deliver them to their corresponding targets.
 */
class Player: public IEntity {
private:
	// the entity the player is current touching
	Platform* _touching;
	// time until the player is invulnerable (millis)
	uint32_t _invulnerableTime;
	// time until the powerup runs out (millis)
	uint32_t _powerUpTime;

	NPCFriendly* _collectedNPC;

	PhysicsVec2 _acceleration;

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
	ConfigVarPtr _maxHitPoints;
	ConfigVarPtr _fruitHitPoints;
	ConfigVarPtr _damageThreshold;
	ConfigVarPtr _amountOfFruitsForANewLife;
	ConfigVarPtr _fruitCollectDelayForANewLife;

	uint32_t _lastFruitCollected;
	uint16_t _hitpoints;
	uint8_t _lives;
	int8_t _fruitsCollectedInARow;

	PhysicsJoint _revoluteJoint;

	PlayerCrashReason _crashReason;

	float getCompleteMass () const;

public:
	Player (Map& map, ClientId clientId);
	virtual ~Player ();

	void resetAcceleration (Direction dir);
	void accelerate (Direction dir);
	void setFingerAcceleration (int dx, int dy);
	void resetFingerAcceleration ();
	void applyForce (const PhysicsVec2& v);

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
	/** Packages currently attached to the player (not yet delivered). */
	int getCollectedPackageCount () const;
	/** 0-based index into currently carried packages; nullptr if out of range. */
	Package* getCollectedPackage (int index) const;

	void subtractHitpoints (uint16_t hitpoints);
	void addHitpoints (uint16_t hitpoints);
	/** Make the player ignore damage until @c _time + durationMillis. */
	void setInvulnerable (uint32_t durationMillis);

	void createBody (const PhysicsVec2 &pos);

	void setCollectedNPC(NPCFriendly *npc);
	void reset ();

	ClientId getClientId () const;
	const std::string& getName () const;
	void setName (const std::string& name);

	// IEntity
	bool shouldApplyWind () const override;
	void update (uint32_t deltaTime) override;
	void onContact (PhysicsContact contact, IEntity* entity) override;
	void onPreSolve (PhysicsContact contact, IEntity* entity, const PhysicsManifold& oldManifold) override;
	bool shouldCollide (const IEntity* entity) const override;
	
	void damageFromHit (PhysicsContact contact, IEntity* entity);
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
	_acceleration = PhysicsVec2_zero;
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
