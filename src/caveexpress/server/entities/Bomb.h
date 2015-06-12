#pragma once

#include "caveexpress/server/entities/CollectableEntity.h"
#include <vector>

namespace caveexpress {

class Bomb: public CollectableEntity {
protected:
	gridCoord _x;
	gridCoord _y;
	float _blastPower;
	int _numRays;
	float _linearDamping;
	typedef std::vector<b2Body*> Particles;
	typedef Particles::iterator ParticlesIter;
	Particles _particles;
	TimerID _explodeTimer;
	TimerID _destroyTimer;

	void destroy();
	void explode();
public:
	Bomb (Map& map, gridCoord x, gridCoord y, const IEntity *creator = nullptr);
	virtual ~Bomb ();

	void createBody ();
	void initiateDetonation ();

	// IEntity
	bool shouldApplyWind () const override;
	bool shouldCollide (const IEntity *entity) const override;
};

}
