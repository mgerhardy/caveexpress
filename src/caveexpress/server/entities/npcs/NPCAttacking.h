#pragma once

#include "caveexpress/server/entities/npcs/NPCAggressive.h"

namespace caveexpress {

// forward decl
class Player;
class Map;

/**
 * @brief Aggressive npc that is moving on a platform and attacks if the player
 * gets to close to the ground.
 * @todo It might happen that these npcs are falling off their platform. Using a joint to prevents
 * this would be a good idea.
 */
class NPCAttacking: public NPCAggressive {
protected:
	void changeAttackingAnimation (const b2Vec2 &targetPos);
	void setAttacking ();

	TimerID _attackTimer;
	TimerID _returnTimer;

	int _platformStartPos;
	int _platformEndPos;

	Player *_attackTarget;

	TimerID startTimer (int length, void(NPCAttacking::*callback) (), bool abortAnimationChanges);
	void walkBack ();

public:
	NPCAttacking (const EntityType& entityType, Map& map, bool right);
	virtual ~NPCAttacking ();

	void calculatePlatformDimensions ();

	void checkAttack (Player *player);
	void attack (Player* player);
	void stopAttack (const Player *player);

	bool returnToInitialPosition () override;

	// NPC
	void update (uint32_t deltaTime) override;

	// IEntity
	void onPreSolve (b2Contact* contact, IEntity* entity, const b2Manifold* oldManifold) override;
};

}
