#pragma once

#include "caveexpress/server/entities/npcs/NPCAggressive.h"

namespace caveexpress {

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
	b2BodyType getBodyType () const override { return b2_kinematicBody; }

	// NPCAggressive
	const Animation& getFallingAnimation () const override;
	bool shouldCollide (const IEntity* entity) const override;
	void onPreSolve (b2Contact* contact, IEntity* entity, const b2Manifold* oldManifold) override;
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
