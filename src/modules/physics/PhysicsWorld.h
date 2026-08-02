#pragma once

#include "physics/PhysicsTypes.h"
#include "physics/PhysicsBody.h"
#include "physics/PhysicsFixture.h"
#include "physics/PhysicsJoint.h"
#include "physics/PhysicsContact.h"
#include "physics/PhysicsCallbacks.h"
#include <memory>

class PhysicsWorld {
public:
	explicit PhysicsWorld (const PhysicsVec2& gravity);
	~PhysicsWorld ();

	PhysicsWorld (const PhysicsWorld&) = delete;
	PhysicsWorld& operator= (const PhysicsWorld&) = delete;

	void step (float timeStep, int velocityIterations, int positionIterations);
	/// Box2D 3.x uses sub-steps; velocity/position iterations are ignored there.
	void step (float timeStep, int subStepCount);

	PhysicsVec2 getGravity () const;
	void setGravity (const PhysicsVec2& gravity);
	void setAutoClearForces (bool flag);

	PhysicsBody createBody (const PhysicsBodyDef& def);
	void destroyBody (PhysicsBody body);

	PhysicsJoint createDistanceJoint (const PhysicsDistanceJointDef& def);
	PhysicsJoint createRevoluteJoint (const PhysicsRevoluteJointDef& def);
	void destroyJoint (PhysicsJoint joint);

	void rayCast (IPhysicsRayCastCallback& callback, const PhysicsVec2& point1, const PhysicsVec2& point2) const;

	void setContactListener (IPhysicsContactListener* listener);
	void setContactFilter (IPhysicsContactFilter* filter);
	void setDestructionListener (IPhysicsDestructionListener* listener);
	void setDebugDraw (IPhysicsDebugDraw* draw, uint32_t flags = 0);
	void debugDraw () const;
	void dump () const;

	/// Backend-private pointer (b2World* or heap world state). Not for game code.
	void* nativeHandle () const { return _impl; }

	static PhysicsWorld* fromBody (PhysicsBody body);

private:
	void* _impl = nullptr;
};

/// Umbrella include for game code.
#include "physics/PhysicsMath.h"
