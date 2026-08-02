#pragma once

#include "IWorldModificator.h"
#include "common/Direction.h"

namespace caveexpress {

/**
 * @brief Applies a wind force to the world
 */
class WindModificator: public IWorldModificator {
private:
	void getRelativePosition (PhysicsVec2& out) const;
	void setForce (float force);
	void applyImpulse (PhysicsBody body, PhysicsVec2 contactPoint, float force) const;

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

	void createBody (const PhysicsVec2 &pos, float shift);

	// IWorldModificator
	void setModificatorState (bool enable) override;
	void setRelativePositionTo (const PhysicsVec2& pos) override;

	// IEntity
	bool shouldCollide (const IEntity* entity) const override;
	void update (uint32_t deltaTime) override;
	void onPreSolve (PhysicsContact contact, IEntity* entity, const PhysicsManifold& oldManifold) override;
};

}
