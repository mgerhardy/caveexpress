#pragma once

#include "caveexpress/server/entities/npcs/NPCAggressive.h"

namespace caveexpress {

/**
 * @brief An aggressive npc that is spawning at the height of the player and
 * is flying straight through the map
 */
class NPCFlying : public NPCAggressive {
private:
	bool _spawnEgg;
public:
	explicit NPCFlying (Map& map);
	virtual ~NPCFlying ();

	void setRemove ();
	void setFlying (const Animation& animation, float speed);

	// NPC
	void onSpawn () override;
	void update (uint32_t deltaTime) override;
	const Animation& getFallingAnimation () const override;
	b2BodyType getBodyType () const override { return b2_kinematicBody; }
	void setDying (const IEntity* entity) override;

	// NPCAggressive
	bool shouldCollide (const IEntity* entity) const override;
	void onPreSolve (b2Contact* contact, IEntity* entity, const b2Manifold* oldManifold) override;
};

inline void NPCFlying::setRemove ()
{
	_remove = true;
}

}
