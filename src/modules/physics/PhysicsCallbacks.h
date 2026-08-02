#pragma once

#include "physics/PhysicsTypes.h"
#include "physics/PhysicsBody.h"
#include "physics/PhysicsFixture.h"
#include "physics/PhysicsContact.h"
#include "physics/PhysicsJoint.h"

class IPhysicsContactListener {
public:
	virtual ~IPhysicsContactListener () {}
	virtual void beginContact (PhysicsContact contact) {}
	virtual void endContact (PhysicsContact contact) {}
	virtual void preSolve (PhysicsContact contact, const PhysicsManifold& oldManifold) {}
	virtual void postSolve (PhysicsContact contact, const PhysicsContactImpulse& impulse) {}
};

class IPhysicsContactFilter {
public:
	virtual ~IPhysicsContactFilter () {}
	virtual bool shouldCollide (PhysicsFixture fixtureA, PhysicsFixture fixtureB) = 0;
};

class IPhysicsDestructionListener {
public:
	virtual ~IPhysicsDestructionListener () {}
	virtual void sayGoodbye (PhysicsJoint joint) {}
	virtual void sayGoodbye (PhysicsFixture fixture) {}
};

class IPhysicsRayCastCallback {
public:
	virtual ~IPhysicsRayCastCallback () {}
	/// Return -1 to filter, 0 to terminate, fraction to clip, 1 to continue.
	virtual float reportFixture (PhysicsFixture fixture, const PhysicsVec2& point,
			const PhysicsVec2& normal, float fraction) = 0;
};

class IPhysicsDebugDraw {
public:
	virtual ~IPhysicsDebugDraw () {}
	virtual void drawPolygon (const PhysicsVec2* vertices, int vertexCount, const PhysicsColor& color) = 0;
	virtual void drawSolidPolygon (const PhysicsVec2* vertices, int vertexCount, const PhysicsColor& color) = 0;
	virtual void drawCircle (const PhysicsVec2& center, float radius, const PhysicsColor& color) = 0;
	virtual void drawSolidCircle (const PhysicsVec2& center, float radius, const PhysicsVec2& axis, const PhysicsColor& color) = 0;
	virtual void drawSegment (const PhysicsVec2& p1, const PhysicsVec2& p2, const PhysicsColor& color) = 0;
	virtual void drawTransform (const PhysicsTransform& xf) = 0;
	virtual void drawPoint (const PhysicsVec2& p, float size, const PhysicsColor& color) = 0;
};

enum PhysicsDebugDrawFlag {
	PhysicsDrawShapes = 1 << 0,
	PhysicsDrawJoints = 1 << 1,
	PhysicsDrawAabb = 1 << 2,
	PhysicsDrawPairs = 1 << 3,
	PhysicsDrawCenterOfMass = 1 << 4
};
