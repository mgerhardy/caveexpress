#pragma once

#include "caveexpress/server/entities/npcs/NPCAggressive.h"

namespace caveexpress {

/**
 * @brief An aggressive npc that will destroy the player on contact
 */
class NPCFish : public NPCAggressive {
private:
	double _magnitude;
	double _amplitude;

	void changeSpeed (float factor);

public:
	NPCFish (Map& map, double magnitude = 0.125, double amplitude = 0.03125);
	virtual ~NPCFish ();

	void setRemove ();
	void setSwimmingAnimation (const Animation& animation);

	// IEntity
	bool isRemove () const override;

	// NPC
	void onSpawn () override;
	float getDensity () const override;
	void update (uint32_t deltaTime) override;
	PhysicsBodyType getBodyType () const override { return PhysicsBodyType::Kinematic; }

	// NPCAggressive
	const Animation& getFallingAnimation () const override;
	bool shouldCollide (const IEntity* entity) const override;
	void onPreSolve (PhysicsContact contact, IEntity* entity, const PhysicsManifold& oldManifold) override;
};

inline void NPCFish::changeSpeed (float factor)
{
	_currentSwimmingSpeed.x *= factor;
}

inline void NPCFish::setRemove ()
{
	_remove = true;
}

}
