#pragma once

#include "common/Compiler.h"
#include "physics/Version.h"
GCC_DIAG_OFF(shadow)
#include <box2d/box2d.h>
GCC_DIAG_ON(shadow)
#include "physics/PhysicsTypes.h"
#include "physics/PhysicsBody.h"
#include "physics/PhysicsFixture.h"
#include "physics/PhysicsJoint.h"
#include "physics/PhysicsContact.h"
#include "physics/PhysicsWorld.h"
#include <unordered_map>
#include <cstdint>

namespace phys24 {

inline PhysicsVec2 toVec (const b2Vec2& v)
{
	return PhysicsVec2(v.x, v.y);
}

inline b2Vec2 toB2 (const PhysicsVec2& v)
{
	return b2Vec2(v.x, v.y);
}

inline PhysicsBody toBody (b2Body* b)
{
	return PhysicsBody::fromStorage(reinterpret_cast<uint64_t>(b));
}

inline b2Body* toB2 (PhysicsBody b)
{
	return b.isValid() ? reinterpret_cast<b2Body*>(static_cast<uintptr_t>(b.key())) : nullptr;
}

inline PhysicsFixture toFixture (b2Fixture* f)
{
	return PhysicsFixture::fromStorage(reinterpret_cast<uint64_t>(f));
}

inline b2Fixture* toB2 (PhysicsFixture f)
{
	return f.isValid() ? reinterpret_cast<b2Fixture*>(static_cast<uintptr_t>(f.key())) : nullptr;
}

inline PhysicsJoint toJoint (b2Joint* j)
{
	return PhysicsJoint::fromStorage(reinterpret_cast<uint64_t>(j));
}

inline b2Joint* toB2 (PhysicsJoint j)
{
	return j.isValid() ? reinterpret_cast<b2Joint*>(static_cast<uintptr_t>(j.key())) : nullptr;
}

inline PhysicsContact toContact (b2Contact* c)
{
	return PhysicsContact::fromStorage(reinterpret_cast<uint64_t>(c));
}

inline b2Contact* toB2 (PhysicsContact c)
{
	return c.isValid() ? reinterpret_cast<b2Contact*>(static_cast<uintptr_t>(c.key())) : nullptr;
}

inline b2BodyType toB2 (PhysicsBodyType t)
{
	switch (t) {
	case PhysicsBodyType::Kinematic:
		return b2_kinematicBody;
	case PhysicsBodyType::Dynamic:
		return b2_dynamicBody;
	case PhysicsBodyType::Static:
	default:
		return b2_staticBody;
	}
}

inline PhysicsShapeType toShapeType (b2Shape::Type t)
{
	switch (t) {
	case b2Shape::e_circle:
		return PhysicsShapeType::Circle;
	case b2Shape::e_edge:
		return PhysicsShapeType::Edge;
	case b2Shape::e_polygon:
		return PhysicsShapeType::Polygon;
	case b2Shape::e_chain:
		return PhysicsShapeType::Chain;
	default:
		return PhysicsShapeType::Polygon;
	}
}

inline PhysicsPointState toPointState (b2PointState s)
{
	switch (s) {
	case b2_addState:
		return PhysicsPointState::Add;
	case b2_persistState:
		return PhysicsPointState::Persist;
	case b2_removeState:
		return PhysicsPointState::Remove;
	case b2_nullState:
	default:
		return PhysicsPointState::Null;
	}
}

inline PhysicsManifold toManifold (const b2Manifold* m)
{
	PhysicsManifold out;
	if (!m)
		return out;
	out.pointCount = m->pointCount;
	for (int i = 0; i < m->pointCount && i < PhysicsMaxManifoldPoints; ++i) {
		out.points[i].normalImpulse = m->points[i].normalImpulse;
		out.points[i].tangentImpulse = m->points[i].tangentImpulse;
	}
	return out;
}

inline b2Filter toB2 (const PhysicsFilter& f)
{
	b2Filter out;
	out.categoryBits = f.categoryBits;
	out.maskBits = f.maskBits;
	out.groupIndex = f.groupIndex;
	return out;
}

inline PhysicsFilter toFilter (const b2Filter& f)
{
	PhysicsFilter out;
	out.categoryBits = f.categoryBits;
	out.maskBits = f.maskBits;
	out.groupIndex = f.groupIndex;
	return out;
}

struct WorldState {
	b2World* world = nullptr;
	PhysicsWorld* owner = nullptr;
	IPhysicsContactListener* contactListener = nullptr;
	IPhysicsContactFilter* contactFilter = nullptr;
	IPhysicsDestructionListener* destructionListener = nullptr;
	IPhysicsDebugDraw* debugDraw = nullptr;
};

inline std::unordered_map<b2World*, WorldState*>& worldMap ()
{
	static std::unordered_map<b2World*, WorldState*> map;
	return map;
}

inline WorldState* stateFor (b2World* w)
{
	auto& map = worldMap();
	const auto it = map.find(w);
	return it == map.end() ? nullptr : it->second;
}

} // namespace phys24
