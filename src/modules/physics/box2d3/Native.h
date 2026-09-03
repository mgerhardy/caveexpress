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
#include <vector>
#include <cstring>
#include <cstdint>

namespace phys3 {

#if !CP_BOX2D_VERSION_ATLEAST(3, 1, 0)
#define B2_MAX_POLYGON_VERTICES b2_maxPolygonVertices
#endif

#if CP_BOX2D_VERSION_ATLEAST(3, 1, 0)
inline uint16_t idGeneration (b2BodyId id) { return id.generation; }
inline uint16_t idGeneration (b2ShapeId id) { return id.generation; }
inline uint16_t idGeneration (b2JointId id) { return id.generation; }
inline void setIdGeneration (b2BodyId& id, uint16_t v) { id.generation = v; }
inline void setIdGeneration (b2ShapeId& id, uint16_t v) { id.generation = v; }
inline void setIdGeneration (b2JointId& id, uint16_t v) { id.generation = v; }
#else
inline uint16_t idGeneration (b2BodyId id) { return id.revision; }
inline uint16_t idGeneration (b2ShapeId id) { return id.revision; }
inline uint16_t idGeneration (b2JointId id) { return id.revision; }
inline void setIdGeneration (b2BodyId& id, uint16_t v) { id.revision = v; }
inline void setIdGeneration (b2ShapeId& id, uint16_t v) { id.revision = v; }
inline void setIdGeneration (b2JointId& id, uint16_t v) { id.revision = v; }
#endif

inline PhysicsVec2 toVec (b2Vec2 v)
{
	return PhysicsVec2(v.x, v.y);
}

inline b2Vec2 toB2 (const PhysicsVec2& v)
{
	return b2Vec2{ v.x, v.y };
}

#if CP_BOX2D_VERSION_ATLEAST(3, 2, 0)
inline PhysicsVec2 fromPos (b2Pos p)
{
	return PhysicsVec2(static_cast<float>(p.x), static_cast<float>(p.y));
}

inline b2Pos toB2Pos (const PhysicsVec2& v)
{
	return b2ToPos(toB2(v));
}
#endif

inline uint64_t packId (int32_t index1, uint16_t generation)
{
	return (uint64_t)(uint32_t)index1 | ((uint64_t)generation << 32);
}

inline void unpackId (uint64_t key, int32_t& index1, uint16_t& generation)
{
	index1 = (int32_t)(uint32_t)(key & 0xffffffffu);
	generation = (uint16_t)(key >> 32);
}

inline PhysicsBody toBody (b2BodyId id)
{
	if (B2_IS_NULL(id))
		return PhysicsBody();
	return PhysicsBody::fromStorage(packId(id.index1, idGeneration(id)), id.world0);
}

inline b2BodyId toB2Body (PhysicsBody b)
{
	b2BodyId id = b2_nullBodyId;
	if (!b.isValid())
		return id;
	uint16_t generation = 0;
	unpackId(b.key(), id.index1, generation);
	setIdGeneration(id, generation);
	id.world0 = (uint16_t)b.aux();
	return id;
}

inline PhysicsFixture toFixture (b2ShapeId id)
{
	if (B2_IS_NULL(id))
		return PhysicsFixture();
	return PhysicsFixture::fromStorage(packId(id.index1, idGeneration(id)), id.world0);
}

inline b2ShapeId toB2Shape (PhysicsFixture f)
{
	b2ShapeId id = b2_nullShapeId;
	if (!f.isValid())
		return id;
	uint16_t generation = 0;
	unpackId(f.key(), id.index1, generation);
	setIdGeneration(id, generation);
	id.world0 = (uint16_t)f.aux();
	return id;
}

inline PhysicsJoint toJoint (b2JointId id)
{
	if (B2_IS_NULL(id))
		return PhysicsJoint();
	return PhysicsJoint::fromStorage(packId(id.index1, idGeneration(id)), id.world0);
}

inline b2JointId toB2Joint (PhysicsJoint j)
{
	b2JointId id = b2_nullJointId;
	if (!j.isValid())
		return id;
	uint16_t generation = 0;
	unpackId(j.key(), id.index1, generation);
	setIdGeneration(id, generation);
	id.world0 = (uint16_t)j.aux();
	return id;
}

/// Contact handle packs shapeA in key/generation and shapeB in aux (world shared).
/// For pre-solve ephemeral contacts, aux high bit of a side channel is used via WorldImpl.
inline PhysicsContact toContact (b2ShapeId a, b2ShapeId b)
{
	if (B2_IS_NULL(a) && B2_IS_NULL(b))
		return PhysicsContact();
	// Pack A into key, B index+gen into aux with world in low bits of a side encoding:
	// key = pack(A), aux = pack(B.index/gen) but we only have 64+64.
	// Use key=pack(A), aux = ((uint64_t)B.index1) | ((uint64_t)B.generation<<32) and require same world0 in A's aux...
	// PhysicsContact has key+aux only. Store world0 in top of...
	// Simpler: key = pack(A.index, A.gen), aux = pack(B.index, B.gen), world must match A.world0==B.world0;
	// recover world0 from shape A by also storing world in unused bits: put world0 in high 16 of nothing.
	// Store: key = pack(A), aux = (uint64_t)A.world0 | (pack(B) << 16) — pack is 64 bits, won't fit.
	//
	// Final scheme:
	//   key  = packId(A.index1, A.generation)
	//   aux  = packId(B.index1, B.generation)  // world0 recovered from A via shape API when needed
	// And we stash world0 by OR into key high... generation is 16 bits in high of key.
	// Add world0 into PhysicsContact by: aux high isn't enough.
	// Store world0 in PhysicsContact by using:
	//   key = pack(A) , aux = pack(B) , and look up world from shape A (world0 is in the id we reconstruct with stored world).
	// We need world0 for both. Store world0 in the unused padding: use fromStorage(pack(A), pack(B)) and
	// keep world0 in a static thread-local during callbacks OR embed world0 replacing assumption A.world0:
	//   Reconstruct A with world0 taken from: (aux_world) — store world0 in bits 48-63 of key by shifting generation only 16 bits...
	// key layout: index1:32 | generation:16 | world0:16
	const uint64_t key = (uint64_t)(uint32_t)a.index1 | ((uint64_t)idGeneration(a) << 32) | ((uint64_t)a.world0 << 48);
	const uint64_t aux = (uint64_t)(uint32_t)b.index1 | ((uint64_t)idGeneration(b) << 32) | ((uint64_t)b.world0 << 48);
	return PhysicsContact::fromStorage(key, aux);
}

inline void fromContact (PhysicsContact c, b2ShapeId& a, b2ShapeId& b)
{
	a = b2_nullShapeId;
	b = b2_nullShapeId;
	if (!c.isValid())
		return;
	a.index1 = (int32_t)(uint32_t)(c.key() & 0xffffffffu);
	setIdGeneration(a, (uint16_t)((c.key() >> 32) & 0xffffu));
	a.world0 = (uint16_t)((c.key() >> 48) & 0xffffu);
	b.index1 = (int32_t)(uint32_t)(c.aux() & 0xffffffffu);
	setIdGeneration(b, (uint16_t)((c.aux() >> 32) & 0xffffu));
	b.world0 = (uint16_t)((c.aux() >> 48) & 0xffffu);
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

inline PhysicsShapeType toShapeType (b2ShapeType t)
{
	switch (t) {
	case b2_circleShape:
		return PhysicsShapeType::Circle;
	case b2_segmentShape:
		return PhysicsShapeType::Edge;
	case b2_polygonShape:
		return PhysicsShapeType::Polygon;
#if !CP_BOX2D_VERSION_ATLEAST(3, 1, 0)
	case b2_smoothSegmentShape:
#else
	case b2_chainSegmentShape:
#endif
		return PhysicsShapeType::Chain;
	case b2_capsuleShape:
		return PhysicsShapeType::Capsule;
	default:
		return PhysicsShapeType::Polygon;
	}
}

inline b2Filter toB2 (const PhysicsFilter& f)
{
	b2Filter out = b2DefaultFilter();
	out.categoryBits = f.categoryBits;
	out.maskBits = f.maskBits;
	out.groupIndex = f.groupIndex;
	return out;
}

inline PhysicsFilter toFilter (b2Filter f)
{
	PhysicsFilter out;
	out.categoryBits = (uint16_t)f.categoryBits;
	out.maskBits = (uint16_t)f.maskBits;
	out.groupIndex = f.groupIndex;
	return out;
}

struct ContactEdgeNode {
	PhysicsContact contact;
	ContactEdgeNode* next = nullptr;
};

struct WorldImpl;

inline std::unordered_map<uint32_t, WorldImpl*>& worldMap ()
{
	static std::unordered_map<uint32_t, WorldImpl*> map;
	return map;
}

struct WorldImpl {
	b2WorldId worldId = b2_nullWorldId;
	PhysicsWorld* owner = nullptr;
	IPhysicsContactListener* contactListener = nullptr;
	IPhysicsContactFilter* contactFilter = nullptr;
	IPhysicsDestructionListener* destructionListener = nullptr;
	IPhysicsDebugDraw* debugDraw = nullptr;
	uint32_t debugFlags = 0;

	// Pre-solve state
	bool preSolveEnabled = true;
	b2ShapeId preShapeA = b2_nullShapeId;
	b2ShapeId preShapeB = b2_nullShapeId;

	// Temporary contact edges for getContactList
	std::vector<ContactEdgeNode> contactEdges;

	~WorldImpl ()
	{
		if (B2_IS_NON_NULL(worldId)) {
			worldMap().erase(worldId.index1);
			b2DestroyWorld(worldId);
			worldId = b2_nullWorldId;
		}
	}
};

inline WorldImpl* stateForWorld (b2WorldId id)
{
	if (B2_IS_NULL(id))
		return nullptr;
	auto& map = worldMap();
	const auto it = map.find(id.index1);
	return it == map.end() ? nullptr : it->second;
}

inline b2WorldId worldOfBody (b2BodyId body)
{
#if !CP_BOX2D_VERSION_ATLEAST(3, 1, 0)
	b2WorldId world = b2_nullWorldId;
	world.index1 = body.world0;
	return world;
#else
	return b2Body_GetWorld(body);
#endif
}

} // namespace phys3
