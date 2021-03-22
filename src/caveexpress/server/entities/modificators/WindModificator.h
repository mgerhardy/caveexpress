#pragma once

#include "IWorldModificator.h"
#include "common/Direction.h"

namespace caveexpress {

/**
 * @brief Applies a wind force to the world
 */
class WindModificator: public IWorldModificator {
private:
	void getRelativePosition (b2Vec2& out) const;
	void setForce (float force);
	void applyImpulse (b2Body* body, b2Vec2 contactPoint, float force) const;

protected:
	bool _state;
	Direction _direction;
	float _shift;
	float _force;
	float _modificatorSize;
	float _beginSizeDivisor;

public:
	WindModificator (Map& map, Direction direction, float force, float size, float beginSizeDivisior = 2.0f);
	virtual ~WindModificator ();

	void createBody (const b2Vec2 &pos, float shift);

	// IWorldModificator
	void setModificatorState (bool enable) override;
	void setRelativePositionTo (const b2Vec2& pos) override;

	// IEntity
	bool shouldCollide (const IEntity* entity) const override;
	void update (uint32_t deltaTime) override;
	void onPreSolve (b2Contact* contact, IEntity* entity, const b2Manifold* oldManifold) override;
};

}
