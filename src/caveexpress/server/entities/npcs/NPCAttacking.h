#pragma once

#include "caveexpress/server/entities/npcs/NPCAggressive.h"

// forward decl
class Player;
class Map;

class NPCAttacking: public NPCAggressive {
protected:
	void changeAttackingAnimation (const b2Vec2 &targetPos);
	void setAttacking ();

	TimerID _attackTimer;
	TimerID _returnTimer;

	int _platformStartPos;
	int _platformEndPos;

	TimerID startTimer (int length, void(NPCAttacking::*callback) (), bool abortAnimationChanges);
	void walkBack ();

protected:
	Player *_attackTarget;

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
