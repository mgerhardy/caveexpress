// API gates use CP_BOX2D_VERSION_ATLEAST from cp-config.h (CMake-detected).
#include "physics/box2d3/Native.h"
#include "physics/PhysicsCallbacks.h"
#include <algorithm>
#include <cmath>

using namespace phys3;

namespace {

inline WorldImpl* implOf (void* p)
{
	return static_cast<WorldImpl*>(p);
}

inline PhysicsColor fromHex (b2HexColor c)
{
	const uint32_t v = (uint32_t)c;
	return PhysicsColor(
			((v >> 16) & 0xff) / 255.0f,
			((v >> 8) & 0xff) / 255.0f,
			(v & 0xff) / 255.0f,
			1.0f);
}

bool customFilterFcn (b2ShapeId shapeIdA, b2ShapeId shapeIdB, void* context)
{
	auto* impl = static_cast<WorldImpl*>(context);
	if (!impl || !impl->contactFilter)
		return true;
	return impl->contactFilter->shouldCollide(toFixture(shapeIdA), toFixture(shapeIdB));
}

#if CP_BOX2D_VERSION_ATLEAST(3, 2, 0)
void preSolveFcn (b2ShapeId shapeIdA, b2ShapeId shapeIdB, b2Manifold* manifold, void* context)
{
	auto* impl = static_cast<WorldImpl*>(context);
	if (!impl || !impl->contactListener)
		return;
	impl->preSolveEnabled = true;
	impl->preShapeA = shapeIdA;
	impl->preShapeB = shapeIdB;
	PhysicsManifold oldManifold;
	impl->contactListener->preSolve(toContact(shapeIdA, shapeIdB), oldManifold);
	if (!impl->preSolveEnabled && manifold != nullptr)
		manifold->pointCount = 0;
}

bool preContinuousFcn (b2ShapeId shapeIdA, b2ShapeId shapeIdB, b2Pos point, b2Vec2 normal, void* context)
{
	(void)shapeIdA;
	(void)shapeIdB;
	(void)point;
	(void)normal;
	auto* impl = static_cast<WorldImpl*>(context);
	return impl == nullptr || impl->preSolveEnabled;
}

float rayCastFcn (b2ShapeId shapeId, b2Pos point, b2Vec2 normal, float fraction, void* context)
{
	auto* cb = static_cast<IPhysicsRayCastCallback*>(context);
	if (!cb)
		return 0.0f;
	return cb->reportFixture(toFixture(shapeId), fromPos(point), toVec(normal), fraction);
}

void drawPolygonFcn (b2WorldTransform transform, const b2Vec2* vertices, int vertexCount, b2HexColor color, void* context)
{
	auto* draw = static_cast<IPhysicsDebugDraw*>(context);
	if (!draw)
		return;
	PhysicsVec2 tmp[B2_MAX_POLYGON_VERTICES];
	const int n = std::min(vertexCount, (int)B2_MAX_POLYGON_VERTICES);
	for (int i = 0; i < n; ++i)
		tmp[i] = fromPos(b2TransformWorldPoint(transform, vertices[i]));
	draw->drawPolygon(tmp, n, fromHex(color));
}

void drawSolidPolygonFcn (b2WorldTransform transform, const b2Vec2* vertices, int vertexCount, float radius, b2HexColor color, void* context)
{
	(void)radius;
	auto* draw = static_cast<IPhysicsDebugDraw*>(context);
	if (!draw)
		return;
	PhysicsVec2 tmp[B2_MAX_POLYGON_VERTICES];
	const int n = std::min(vertexCount, (int)B2_MAX_POLYGON_VERTICES);
	for (int i = 0; i < n; ++i)
		tmp[i] = fromPos(b2TransformWorldPoint(transform, vertices[i]));
	draw->drawSolidPolygon(tmp, n, fromHex(color));
}

void drawCircleFcn (b2Pos center, float radius, b2HexColor color, void* context)
{
	auto* draw = static_cast<IPhysicsDebugDraw*>(context);
	if (draw)
		draw->drawCircle(fromPos(center), radius, fromHex(color));
}

void drawSolidCircleFcn (b2WorldTransform transform, b2Vec2 center, float radius, b2HexColor color, void* context)
{
	auto* draw = static_cast<IPhysicsDebugDraw*>(context);
	if (!draw)
		return;
	const PhysicsVec2 worldCenter = fromPos(b2TransformWorldPoint(transform, center));
	PhysicsVec2 xAxis(transform.q.c, transform.q.s);
	draw->drawSolidCircle(worldCenter, radius, xAxis, fromHex(color));
}

void drawLineFcn (b2Pos p1, b2Pos p2, b2HexColor color, void* context)
{
	auto* draw = static_cast<IPhysicsDebugDraw*>(context);
	if (draw)
		draw->drawSegment(fromPos(p1), fromPos(p2), fromHex(color));
}

void drawTransformFcn (b2WorldTransform transform, void* context)
{
	auto* draw = static_cast<IPhysicsDebugDraw*>(context);
	if (!draw)
		return;
	PhysicsTransform t;
	t.p = fromPos(transform.p);
	t.c = transform.q.c;
	t.s = transform.q.s;
	draw->drawTransform(t);
}

void drawPointFcn (b2Pos p, float size, b2HexColor color, void* context)
{
	auto* draw = static_cast<IPhysicsDebugDraw*>(context);
	if (draw)
		draw->drawPoint(fromPos(p), size, fromHex(color));
}
#else
bool preSolveFcn (b2ShapeId shapeIdA, b2ShapeId shapeIdB, b2Manifold* manifold, void* context)
{
	(void)manifold;
	auto* impl = static_cast<WorldImpl*>(context);
	if (!impl || !impl->contactListener)
		return true;
	impl->preSolveEnabled = true;
	impl->preShapeA = shapeIdA;
	impl->preShapeB = shapeIdB;
	PhysicsManifold oldManifold;
	impl->contactListener->preSolve(toContact(shapeIdA, shapeIdB), oldManifold);
	return impl->preSolveEnabled;
}

float rayCastFcn (b2ShapeId shapeId, b2Vec2 point, b2Vec2 normal, float fraction, void* context)
{
	auto* cb = static_cast<IPhysicsRayCastCallback*>(context);
	if (!cb)
		return 0.0f;
	return cb->reportFixture(toFixture(shapeId), toVec(point), toVec(normal), fraction);
}

void drawPolygonFcn (const b2Vec2* vertices, int vertexCount, b2HexColor color, void* context)
{
	auto* draw = static_cast<IPhysicsDebugDraw*>(context);
	if (!draw)
		return;
	PhysicsVec2 tmp[B2_MAX_POLYGON_VERTICES];
	const int n = std::min(vertexCount, (int)B2_MAX_POLYGON_VERTICES);
	for (int i = 0; i < n; ++i)
		tmp[i] = toVec(vertices[i]);
	draw->drawPolygon(tmp, n, fromHex(color));
}

void drawSolidPolygonFcn (b2Transform transform, const b2Vec2* vertices, int vertexCount, float radius, b2HexColor color, void* context)
{
	(void)radius;
	auto* draw = static_cast<IPhysicsDebugDraw*>(context);
	if (!draw)
		return;
	PhysicsVec2 tmp[B2_MAX_POLYGON_VERTICES];
	const int n = std::min(vertexCount, (int)B2_MAX_POLYGON_VERTICES);
	for (int i = 0; i < n; ++i)
		tmp[i] = toVec(b2TransformPoint(transform, vertices[i]));
	draw->drawSolidPolygon(tmp, n, fromHex(color));
}

void drawCircleFcn (b2Vec2 center, float radius, b2HexColor color, void* context)
{
	auto* draw = static_cast<IPhysicsDebugDraw*>(context);
	if (draw)
		draw->drawCircle(toVec(center), radius, fromHex(color));
}

void drawSolidCircleFcn (b2Transform transform, float radius, b2HexColor color, void* context)
{
	auto* draw = static_cast<IPhysicsDebugDraw*>(context);
	if (!draw)
		return;
	PhysicsVec2 xAxis(transform.q.c, transform.q.s);
	draw->drawSolidCircle(toVec(transform.p), radius, xAxis, fromHex(color));
}

void drawSegmentFcn (b2Vec2 p1, b2Vec2 p2, b2HexColor color, void* context)
{
	auto* draw = static_cast<IPhysicsDebugDraw*>(context);
	if (draw)
		draw->drawSegment(toVec(p1), toVec(p2), fromHex(color));
}

void drawTransformFcn (b2Transform transform, void* context)
{
	auto* draw = static_cast<IPhysicsDebugDraw*>(context);
	if (!draw)
		return;
	PhysicsTransform t;
	t.p = toVec(transform.p);
	t.c = transform.q.c;
	t.s = transform.q.s;
	draw->drawTransform(t);
}

void drawPointFcn (b2Vec2 p, float size, b2HexColor color, void* context)
{
	auto* draw = static_cast<IPhysicsDebugDraw*>(context);
	if (draw)
		draw->drawPoint(toVec(p), size, fromHex(color));
}
#endif

void drainEvents (WorldImpl* impl)
{
	if (!impl->contactListener)
		return;

	const b2ContactEvents contacts = b2World_GetContactEvents(impl->worldId);
	for (int i = 0; i < contacts.beginCount; ++i) {
		const b2ContactBeginTouchEvent& e = contacts.beginEvents[i];
		impl->contactListener->beginContact(toContact(e.shapeIdA, e.shapeIdB));
	}
	for (int i = 0; i < contacts.endCount; ++i) {
		const b2ContactEndTouchEvent& e = contacts.endEvents[i];
		impl->contactListener->endContact(toContact(e.shapeIdA, e.shapeIdB));
	}

	const b2SensorEvents sensors = b2World_GetSensorEvents(impl->worldId);
	for (int i = 0; i < sensors.beginCount; ++i) {
		const b2SensorBeginTouchEvent& e = sensors.beginEvents[i];
		impl->contactListener->beginContact(toContact(e.sensorShapeId, e.visitorShapeId));
	}
	for (int i = 0; i < sensors.endCount; ++i) {
		const b2SensorEndTouchEvent& e = sensors.endEvents[i];
		impl->contactListener->endContact(toContact(e.sensorShapeId, e.visitorShapeId));
	}
}

b2ShapeDef makeShapeDef (const PhysicsFixtureDef& def)
{
	b2ShapeDef sd = b2DefaultShapeDef();
	sd.density = def.density;
#if !CP_BOX2D_VERSION_ATLEAST(3, 1, 0)
	sd.friction = def.friction;
	sd.restitution = def.restitution;
#else
	sd.material.friction = def.friction;
	sd.material.restitution = def.restitution;
#endif
	sd.isSensor = def.isSensor;
	sd.filter = toB2(def.filter);
	sd.userData = reinterpret_cast<void*>(def.userData);
	sd.enableContactEvents = def.enableContactEvents;
	sd.enablePreSolveEvents = def.enablePreSolveEvents;
#if CP_BOX2D_VERSION_ATLEAST(3, 2, 0)
	sd.enableCustomFiltering = true;
#endif
	sd.enableSensorEvents = def.isSensor || def.enableContactEvents;
	return sd;
}

} // namespace

// ---------------------------------------------------------------------------
// PhysicsWorld
// ---------------------------------------------------------------------------

PhysicsWorld::PhysicsWorld (const PhysicsVec2& gravity)
{
	auto* impl = new WorldImpl();
	b2WorldDef def = b2DefaultWorldDef();
	def.gravity = toB2(gravity);
	impl->worldId = b2CreateWorld(&def);
	impl->owner = this;
	worldMap()[impl->worldId.index1] = impl;
	_impl = impl;
}

PhysicsWorld::~PhysicsWorld ()
{
	delete implOf(_impl);
	_impl = nullptr;
}

void PhysicsWorld::step (float timeStep, int velocityIterations, int positionIterations)
{
	(void)velocityIterations;
	(void)positionIterations;
	step(timeStep, 4);
}

void PhysicsWorld::step (float timeStep, int subStepCount)
{
	WorldImpl* impl = implOf(_impl);
	b2World_Step(impl->worldId, timeStep, subStepCount);
	drainEvents(impl);
}

PhysicsVec2 PhysicsWorld::getGravity () const
{
	return toVec(b2World_GetGravity(implOf(_impl)->worldId));
}

void PhysicsWorld::setGravity (const PhysicsVec2& gravity)
{
	b2World_SetGravity(implOf(_impl)->worldId, toB2(gravity));
}

void PhysicsWorld::setAutoClearForces (bool flag)
{
	(void)flag; // always cleared each step in 3.x
}

PhysicsBody PhysicsWorld::createBody (const PhysicsBodyDef& def)
{
	b2BodyDef bd = b2DefaultBodyDef();
	bd.type = toB2(def.type);
#if CP_BOX2D_VERSION_ATLEAST(3, 2, 0)
	bd.position = toB2Pos(def.position);
	if (def.fixedRotation)
		bd.motionLocks.angularZ = true;
#else
	bd.position = toB2(def.position);
	bd.fixedRotation = def.fixedRotation;
#endif
	bd.rotation = b2MakeRot(def.angle);
	bd.linearVelocity = toB2(def.linearVelocity);
	bd.angularVelocity = def.angularVelocity;
	bd.linearDamping = def.linearDamping;
	bd.angularDamping = def.angularDamping;
	bd.gravityScale = def.gravityScale;
	bd.isBullet = def.bullet;
	bd.userData = reinterpret_cast<void*>(def.userData);
	return toBody(b2CreateBody(implOf(_impl)->worldId, &bd));
}

void PhysicsWorld::destroyBody (PhysicsBody body)
{
	const b2BodyId id = toB2Body(body);
	if (B2_IS_NON_NULL(id))
		b2DestroyBody(id);
}

PhysicsJoint PhysicsWorld::createDistanceJoint (const PhysicsDistanceJointDef& def)
{
	b2DistanceJointDef jd = b2DefaultDistanceJointDef();
#if CP_BOX2D_VERSION_ATLEAST(3, 2, 0)
	jd.base.bodyIdA = toB2Body(def.bodyA);
	jd.base.bodyIdB = toB2Body(def.bodyB);
	if (def.useWorldAnchors) {
		jd.base.localFrameA.p = b2Body_GetLocalPoint(jd.base.bodyIdA, toB2Pos(def.worldAnchorA));
		jd.base.localFrameB.p = b2Body_GetLocalPoint(jd.base.bodyIdB, toB2Pos(def.worldAnchorB));
	} else {
		jd.base.localFrameA.p = toB2(def.localAnchorA);
		jd.base.localFrameB.p = toB2(def.localAnchorB);
	}
	jd.base.localFrameA.q = b2Rot_identity;
	jd.base.localFrameB.q = b2Rot_identity;
	jd.base.collideConnected = def.collideConnected;
	jd.base.userData = reinterpret_cast<void*>(def.userData);
#else
	jd.bodyIdA = toB2Body(def.bodyA);
	jd.bodyIdB = toB2Body(def.bodyB);
	if (def.useWorldAnchors) {
		jd.localAnchorA = b2Body_GetLocalPoint(jd.bodyIdA, toB2(def.worldAnchorA));
		jd.localAnchorB = b2Body_GetLocalPoint(jd.bodyIdB, toB2(def.worldAnchorB));
	} else {
		jd.localAnchorA = toB2(def.localAnchorA);
		jd.localAnchorB = toB2(def.localAnchorB);
	}
	jd.collideConnected = def.collideConnected;
	jd.userData = reinterpret_cast<void*>(def.userData);
#endif
	jd.length = 0.5f * (def.minLength + def.maxLength);
	jd.enableSpring = false;
	jd.enableLimit = true;
	jd.minLength = def.minLength;
	jd.maxLength = def.maxLength;
	return toJoint(b2CreateDistanceJoint(implOf(_impl)->worldId, &jd));
}

PhysicsJoint PhysicsWorld::createRevoluteJoint (const PhysicsRevoluteJointDef& def)
{
	b2RevoluteJointDef jd = b2DefaultRevoluteJointDef();
#if CP_BOX2D_VERSION_ATLEAST(3, 2, 0)
	jd.base.bodyIdA = toB2Body(def.bodyA);
	jd.base.bodyIdB = toB2Body(def.bodyB);
	if (def.useWorldPivot) {
		jd.base.localFrameA.p = b2Body_GetLocalPoint(jd.base.bodyIdA, toB2Pos(def.worldPivot));
		jd.base.localFrameB.p = b2Body_GetLocalPoint(jd.base.bodyIdB, toB2Pos(def.worldPivot));
	} else {
		jd.base.localFrameA.p = toB2(def.localAnchorA);
		jd.base.localFrameB.p = toB2(def.localAnchorB);
	}
	jd.base.localFrameA.q = b2Rot_identity;
	jd.base.localFrameB.q = b2Rot_identity;
	jd.base.collideConnected = def.collideConnected;
	jd.base.userData = reinterpret_cast<void*>(def.userData);
#else
	jd.bodyIdA = toB2Body(def.bodyA);
	jd.bodyIdB = toB2Body(def.bodyB);
	if (def.useWorldPivot) {
		jd.localAnchorA = b2Body_GetLocalPoint(jd.bodyIdA, toB2(def.worldPivot));
		jd.localAnchorB = b2Body_GetLocalPoint(jd.bodyIdB, toB2(def.worldPivot));
	} else {
		jd.localAnchorA = toB2(def.localAnchorA);
		jd.localAnchorB = toB2(def.localAnchorB);
	}
	jd.collideConnected = def.collideConnected;
	jd.userData = reinterpret_cast<void*>(def.userData);
#endif
	jd.lowerAngle = def.lowerAngle;
	jd.upperAngle = def.upperAngle;
	jd.enableLimit = def.enableLimit;
	jd.enableMotor = def.enableMotor;
	jd.motorSpeed = def.motorSpeed;
	jd.maxMotorTorque = def.maxMotorTorque;
	return toJoint(b2CreateRevoluteJoint(implOf(_impl)->worldId, &jd));
}

void PhysicsWorld::destroyJoint (PhysicsJoint joint)
{
	const b2JointId id = toB2Joint(joint);
	if (B2_IS_NON_NULL(id))
		b2DestroyJoint(id);
}

void PhysicsWorld::rayCast (IPhysicsRayCastCallback& callback, const PhysicsVec2& point1, const PhysicsVec2& point2) const
{
	const b2Vec2 translation = toB2(point2 - point1);
#if CP_BOX2D_VERSION_ATLEAST(3, 2, 0)
	b2World_CastRay(implOf(_impl)->worldId, toB2Pos(point1), translation, b2DefaultQueryFilter(), rayCastFcn, &callback);
#else
	b2World_CastRay(implOf(_impl)->worldId, toB2(point1), translation, b2DefaultQueryFilter(), rayCastFcn, &callback);
#endif
}

void PhysicsWorld::setContactListener (IPhysicsContactListener* listener)
{
	WorldImpl* impl = implOf(_impl);
	impl->contactListener = listener;
#if CP_BOX2D_VERSION_ATLEAST(3, 2, 0)
	b2World_SetPreSolveCallback(impl->worldId, listener ? preSolveFcn : nullptr,
			listener ? preContinuousFcn : nullptr, impl);
#else
	b2World_SetPreSolveCallback(impl->worldId, listener ? preSolveFcn : nullptr, impl);
#endif
}

void PhysicsWorld::setContactFilter (IPhysicsContactFilter* filter)
{
	WorldImpl* impl = implOf(_impl);
	impl->contactFilter = filter;
	b2World_SetCustomFilterCallback(impl->worldId, filter ? customFilterFcn : nullptr, impl);
}

void PhysicsWorld::setDestructionListener (IPhysicsDestructionListener* listener)
{
	implOf(_impl)->destructionListener = listener;
}

void PhysicsWorld::setDebugDraw (IPhysicsDebugDraw* draw, uint32_t flags)
{
	WorldImpl* impl = implOf(_impl);
	impl->debugDraw = draw;
	impl->debugFlags = flags ? flags
			: (PhysicsDrawShapes | PhysicsDrawJoints | PhysicsDrawAabb | PhysicsDrawPairs | PhysicsDrawCenterOfMass);
}

void PhysicsWorld::debugDraw () const
{
	WorldImpl* impl = implOf(_impl);
	if (!impl->debugDraw)
		return;
#if !CP_BOX2D_VERSION_ATLEAST(3, 1, 0)
	b2DebugDraw dd = {};
	dd.context = impl->debugDraw;
	dd.DrawPolygon = drawPolygonFcn;
	dd.DrawSolidPolygon = drawSolidPolygonFcn;
	dd.DrawCircle = drawCircleFcn;
	dd.DrawSolidCircle = drawSolidCircleFcn;
	dd.DrawSegment = drawSegmentFcn;
	dd.DrawTransform = drawTransformFcn;
	dd.DrawPoint = drawPointFcn;
	dd.drawShapes = (impl->debugFlags & PhysicsDrawShapes) != 0;
	dd.drawJoints = (impl->debugFlags & PhysicsDrawJoints) != 0;
	dd.drawAABBs = (impl->debugFlags & PhysicsDrawAabb) != 0;
	dd.drawMass = (impl->debugFlags & PhysicsDrawCenterOfMass) != 0;
#else
	b2DebugDraw dd = b2DefaultDebugDraw();
	dd.context = impl->debugDraw;
	dd.DrawPolygonFcn = drawPolygonFcn;
	dd.DrawSolidPolygonFcn = drawSolidPolygonFcn;
	dd.DrawCircleFcn = drawCircleFcn;
	dd.DrawSolidCircleFcn = drawSolidCircleFcn;
#if CP_BOX2D_VERSION_ATLEAST(3, 2, 0)
	dd.DrawLineFcn = drawLineFcn;
#else
	dd.DrawSegmentFcn = drawSegmentFcn;
#endif
	dd.DrawTransformFcn = drawTransformFcn;
	dd.DrawPointFcn = drawPointFcn;
	dd.drawShapes = (impl->debugFlags & PhysicsDrawShapes) != 0;
	dd.drawJoints = (impl->debugFlags & PhysicsDrawJoints) != 0;
	dd.drawBounds = (impl->debugFlags & PhysicsDrawAabb) != 0;
	dd.drawMass = (impl->debugFlags & PhysicsDrawCenterOfMass) != 0;
#endif
	b2World_Draw(impl->worldId, &dd);
}

void PhysicsWorld::dump () const
{
	// No Dump() in 3.x
}

PhysicsWorld* PhysicsWorld::fromBody (PhysicsBody body)
{
	const b2BodyId id = toB2Body(body);
	if (B2_IS_NULL(id))
		return nullptr;
	WorldImpl* st = stateForWorld(worldOfBody(id));
	return st ? st->owner : nullptr;
}

// ---------------------------------------------------------------------------
// PhysicsBody
// ---------------------------------------------------------------------------

PhysicsVec2 PhysicsBody::getPosition () const
{
#if CP_BOX2D_VERSION_ATLEAST(3, 2, 0)
	return fromPos(b2Body_GetPosition(toB2Body(*this)));
#else
	return toVec(b2Body_GetPosition(toB2Body(*this)));
#endif
}

float PhysicsBody::getAngle () const
{
	return b2Rot_GetAngle(b2Body_GetRotation(toB2Body(*this)));
}

PhysicsVec2 PhysicsBody::getWorldCenter () const
{
#if CP_BOX2D_VERSION_ATLEAST(3, 2, 0)
	return fromPos(b2Body_GetWorldCenter(toB2Body(*this)));
#else
	return toVec(b2Body_GetWorldCenterOfMass(toB2Body(*this)));
#endif
}

PhysicsVec2 PhysicsBody::getWorldPoint (const PhysicsVec2& localPoint) const
{
#if CP_BOX2D_VERSION_ATLEAST(3, 2, 0)
	return fromPos(b2Body_GetWorldPoint(toB2Body(*this), toB2(localPoint)));
#else
	return toVec(b2Body_GetWorldPoint(toB2Body(*this), toB2(localPoint)));
#endif
}

PhysicsVec2 PhysicsBody::getLinearVelocity () const
{
	return toVec(b2Body_GetLinearVelocity(toB2Body(*this)));
}

float PhysicsBody::getAngularVelocity () const
{
	return b2Body_GetAngularVelocity(toB2Body(*this));
}

float PhysicsBody::getInertia () const
{
#if !CP_BOX2D_VERSION_ATLEAST(3, 1, 0)
	return b2Body_GetInertiaTensor(toB2Body(*this));
#else
	return b2Body_GetRotationalInertia(toB2Body(*this));
#endif
}

float PhysicsBody::getMass () const
{
	return b2Body_GetMass(toB2Body(*this));
}

float PhysicsBody::getGravityScale () const
{
	return b2Body_GetGravityScale(toB2Body(*this));
}

PhysicsVec2 PhysicsBody::getLinearVelocityFromWorldPoint (const PhysicsVec2& worldPoint) const
{
#if !CP_BOX2D_VERSION_ATLEAST(3, 1, 0)
	const b2BodyId body = toB2Body(*this);
	const b2Vec2 v = b2Body_GetLinearVelocity(body);
	const float w = b2Body_GetAngularVelocity(body);
	const b2Vec2 c = b2Body_GetWorldCenterOfMass(body);
	const float rx = worldPoint.x - c.x;
	const float ry = worldPoint.y - c.y;
	return PhysicsVec2(v.x - w * ry, v.y + w * rx);
#elif CP_BOX2D_VERSION_ATLEAST(3, 2, 0)
	return toVec(b2Body_GetWorldPointVelocity(toB2Body(*this), toB2Pos(worldPoint)));
#else
	return toVec(b2Body_GetWorldPointVelocity(toB2Body(*this), toB2(worldPoint)));
#endif
}

PhysicsUserData PhysicsBody::getUserData () const
{
	return reinterpret_cast<PhysicsUserData>(b2Body_GetUserData(toB2Body(*this)));
}

PhysicsWorld* PhysicsBody::getWorld () const
{
	return PhysicsWorld::fromBody(*this);
}

void PhysicsBody::setTransform (const PhysicsVec2& position, float angle) const {
#if CP_BOX2D_VERSION_ATLEAST(3, 2, 0)
	b2Body_SetTransform(toB2Body(*this), toB2Pos(position), b2MakeRot(angle));
#else
	b2Body_SetTransform(toB2Body(*this), toB2(position), b2MakeRot(angle));
#endif
}

void PhysicsBody::setLinearVelocity (const PhysicsVec2& v) const {
	b2Body_SetLinearVelocity(toB2Body(*this), toB2(v));
}

void PhysicsBody::setAngularVelocity (float omega) const {
	b2Body_SetAngularVelocity(toB2Body(*this), omega);
}

void PhysicsBody::setGravityScale (float scale) const {
	b2Body_SetGravityScale(toB2Body(*this), scale);
}

void PhysicsBody::setLinearDamping (float damping) const {
	b2Body_SetLinearDamping(toB2Body(*this), damping);
}

void PhysicsBody::setAngularDamping (float damping) const {
	b2Body_SetAngularDamping(toB2Body(*this), damping);
}

void PhysicsBody::setFixedRotation (bool flag) const {
#if CP_BOX2D_VERSION_ATLEAST(3, 2, 0)
	b2MotionLocks locks = b2Body_GetMotionLocks(toB2Body(*this));
	locks.angularZ = flag;
	b2Body_SetMotionLocks(toB2Body(*this), locks);
#else
	b2Body_SetFixedRotation(toB2Body(*this), flag);
#endif
}

void PhysicsBody::setEnabled (bool flag) const {
	if (flag)
		b2Body_Enable(toB2Body(*this));
	else
		b2Body_Disable(toB2Body(*this));
}

void PhysicsBody::setUserData (PhysicsUserData data) const {
	b2Body_SetUserData(toB2Body(*this), reinterpret_cast<void*>(data));
}

void PhysicsBody::applyForce (const PhysicsVec2& force, const PhysicsVec2& point, bool wake) const {
#if CP_BOX2D_VERSION_ATLEAST(3, 2, 0)
	b2Body_ApplyForce(toB2Body(*this), toB2(force), toB2Pos(point), wake);
#else
	b2Body_ApplyForce(toB2Body(*this), toB2(force), toB2(point), wake);
#endif
}

void PhysicsBody::applyForceToCenter (const PhysicsVec2& force, bool wake) const {
	b2Body_ApplyForceToCenter(toB2Body(*this), toB2(force), wake);
}

void PhysicsBody::applyLinearImpulse (const PhysicsVec2& impulse, const PhysicsVec2& point, bool wake) const {
#if CP_BOX2D_VERSION_ATLEAST(3, 2, 0)
	b2Body_ApplyLinearImpulse(toB2Body(*this), toB2(impulse), toB2Pos(point), wake);
#else
	b2Body_ApplyLinearImpulse(toB2Body(*this), toB2(impulse), toB2(point), wake);
#endif
}

void PhysicsBody::applyTorque (float torque, bool wake) const {
	b2Body_ApplyTorque(toB2Body(*this), torque, wake);
}

PhysicsFixture PhysicsBody::createFixture (const PhysicsFixtureDef& def) const {
	b2ShapeDef sd = makeShapeDef(def);
	b2ShapeId shapeId = b2_nullShapeId;
	const b2BodyId bodyId = toB2Body(*this);

	switch (def.shapeType) {
	case PhysicsShapeType::Circle: {
		b2Circle circle;
		circle.center = toB2(def.circleCenter);
		circle.radius = def.radius;
		shapeId = b2CreateCircleShape(bodyId, &sd, &circle);
		break;
	}
	case PhysicsShapeType::Edge: {
		b2Segment segment;
		segment.point1 = toB2(def.vertices[0]);
		segment.point2 = toB2(def.vertices[1]);
		shapeId = b2CreateSegmentShape(bodyId, &sd, &segment);
		break;
	}
	case PhysicsShapeType::Polygon:
	default: {
		b2Polygon polygon;
		if (def.useBox) {
			polygon = b2MakeBox(def.boxHalfWidth, def.boxHalfHeight);
		} else {
			b2Vec2 verts[B2_MAX_POLYGON_VERTICES];
			const int n = std::min(def.vertexCount, (int)B2_MAX_POLYGON_VERTICES);
			for (int i = 0; i < n; ++i)
				verts[i] = toB2(def.vertices[i]);
			const b2Hull hull = b2ComputeHull(verts, n);
			polygon = b2MakePolygon(&hull, 0.0f);
		}
		shapeId = b2CreatePolygonShape(bodyId, &sd, &polygon);
		break;
	}
	}
	return toFixture(shapeId);
}

PhysicsFixture PhysicsBody::getFixtureList () const
{
	b2ShapeId shapes[16];
	const int count = b2Body_GetShapes(toB2Body(*this), shapes, 16);
	if (count <= 0)
		return PhysicsFixture();
	return toFixture(shapes[0]);
}

PhysicsContactEdge PhysicsBody::getContactList () const
{
	WorldImpl* impl = stateForWorld(worldOfBody(toB2Body(*this)));
	if (!impl)
		return PhysicsContactEdge();

	b2ContactData data[32];
	const int count = b2Body_GetContactData(toB2Body(*this), data, 32);
	impl->contactEdges.clear();
	impl->contactEdges.resize(count);
	for (int i = 0; i < count; ++i) {
		impl->contactEdges[i].contact = toContact(data[i].shapeIdA, data[i].shapeIdB);
		impl->contactEdges[i].next = (i + 1 < count) ? &impl->contactEdges[i + 1] : nullptr;
	}
	if (count == 0)
		return PhysicsContactEdge();
	PhysicsContactEdge edge = PhysicsContactEdge::fromStorage(reinterpret_cast<uint64_t>(&impl->contactEdges[0]));
	edge.contact = impl->contactEdges[0].contact;
	return edge;
}

// ---------------------------------------------------------------------------
// PhysicsFixture
// ---------------------------------------------------------------------------

PhysicsBody PhysicsFixture::getBody () const
{
	return toBody(b2Shape_GetBody(toB2Shape(*this)));
}

PhysicsShapeType PhysicsFixture::getShapeType () const
{
	return toShapeType(b2Shape_GetType(toB2Shape(*this)));
}

float PhysicsFixture::getDensity () const
{
	return b2Shape_GetDensity(toB2Shape(*this));
}

PhysicsFilter PhysicsFixture::getFilterData () const
{
	return toFilter(b2Shape_GetFilter(toB2Shape(*this)));
}

PhysicsUserData PhysicsFixture::getUserData () const
{
	return reinterpret_cast<PhysicsUserData>(b2Shape_GetUserData(toB2Shape(*this)));
}

void PhysicsFixture::setUserData (PhysicsUserData data) const {
	b2Shape_SetUserData(toB2Shape(*this), reinterpret_cast<void*>(data));
}

void PhysicsFixture::refilter () const {
	// Re-apply filter to force contact rebuild
	const b2Filter f = b2Shape_GetFilter(toB2Shape(*this));
	b2Shape_SetFilter(toB2Shape(*this), f);
}

PhysicsFixture PhysicsFixture::getNext () const
{
	const b2ShapeId self = toB2Shape(*this);
	const b2BodyId body = b2Shape_GetBody(self);
	b2ShapeId shapes[32];
	const int count = b2Body_GetShapes(body, shapes, 32);
	for (int i = 0; i < count - 1; ++i) {
		if (B2_ID_EQUALS(shapes[i], self))
			return toFixture(shapes[i + 1]);
	}
	return PhysicsFixture();
}

int PhysicsFixture::getPolygonVertexCount () const
{
	if (b2Shape_GetType(toB2Shape(*this)) != b2_polygonShape)
		return 0;
	return b2Shape_GetPolygon(toB2Shape(*this)).count;
}

PhysicsVec2 PhysicsFixture::getPolygonVertex (int index) const
{
	return toVec(b2Shape_GetPolygon(toB2Shape(*this)).vertices[index]);
}

float PhysicsFixture::getCircleRadius () const
{
	if (b2Shape_GetType(toB2Shape(*this)) != b2_circleShape)
		return 0.0f;
	return b2Shape_GetCircle(toB2Shape(*this)).radius;
}

PhysicsVec2 PhysicsFixture::getCircleLocalCenter () const
{
	if (b2Shape_GetType(toB2Shape(*this)) != b2_circleShape)
		return PhysicsVec2_zero;
	return toVec(b2Shape_GetCircle(toB2Shape(*this)).center);
}

int PhysicsFixture::getChildCount () const
{
	return 1;
}

void PhysicsFixture::computeAABB (PhysicsAABB& aabb, const PhysicsTransform& xf, int childIndex) const
{
	(void)xf;
	(void)childIndex;
	const b2AABB box = b2Shape_GetAABB(toB2Shape(*this));
	aabb.lowerBound = toVec(box.lowerBound);
	aabb.upperBound = toVec(box.upperBound);
}

// ---------------------------------------------------------------------------
// PhysicsJoint
// ---------------------------------------------------------------------------

PhysicsBody PhysicsJoint::getBodyA () const
{
	return toBody(b2Joint_GetBodyA(toB2Joint(*this)));
}

PhysicsBody PhysicsJoint::getBodyB () const
{
	return toBody(b2Joint_GetBodyB(toB2Joint(*this)));
}

PhysicsUserData PhysicsJoint::getUserData () const
{
	return reinterpret_cast<PhysicsUserData>(b2Joint_GetUserData(toB2Joint(*this)));
}

void PhysicsJoint::setUserData (PhysicsUserData data) const {
	b2Joint_SetUserData(toB2Joint(*this), reinterpret_cast<void*>(data));
}

float PhysicsJoint::getLength () const
{
	return b2DistanceJoint_GetLength(toB2Joint(*this));
}

void PhysicsJoint::setLength (float length) const {
	b2DistanceJoint_SetLength(toB2Joint(*this), length);
}

void PhysicsJoint::setMotorSpeed (float speed) const {
	b2RevoluteJoint_SetMotorSpeed(toB2Joint(*this), speed);
}

// ---------------------------------------------------------------------------
// PhysicsContact
// ---------------------------------------------------------------------------

PhysicsFixture PhysicsContact::getFixtureA () const
{
	b2ShapeId a, b;
	fromContact(*this, a, b);
	return toFixture(a);
}

PhysicsFixture PhysicsContact::getFixtureB () const
{
	b2ShapeId a, b;
	fromContact(*this, a, b);
	return toFixture(b);
}

PhysicsManifold PhysicsContact::getManifold () const
{
	return PhysicsManifold();
}

void PhysicsContact::getWorldManifold (PhysicsWorldManifold& manifold) const
{
	manifold = PhysicsWorldManifold();
	b2ShapeId a, b;
	fromContact(*this, a, b);
	if (B2_IS_NULL(a) || B2_IS_NULL(b))
		return;
	// Approximate normal from centers
	const b2BodyId bodyA = b2Shape_GetBody(a);
	const b2BodyId bodyB = b2Shape_GetBody(b);
#if CP_BOX2D_VERSION_ATLEAST(3, 2, 0)
	const b2Pos ca = b2Body_GetWorldCenter(bodyA);
	const b2Pos cb = b2Body_GetWorldCenter(bodyB);
	b2Vec2 n = { static_cast<float>(cb.x - ca.x), static_cast<float>(cb.y - ca.y) };
#else
	const b2Vec2 ca = b2Body_GetWorldCenterOfMass(bodyA);
	const b2Vec2 cb = b2Body_GetWorldCenterOfMass(bodyB);
	b2Vec2 n = { cb.x - ca.x, cb.y - ca.y };
#endif
	const float len = sqrtf(n.x * n.x + n.y * n.y);
	if (len > 0.0f) {
		n.x /= len;
		n.y /= len;
	}
	manifold.normal = toVec(n);
	manifold.points[0] = PhysicsVec2(0.5f * static_cast<float>(ca.x + cb.x), 0.5f * static_cast<float>(ca.y + cb.y));
}

bool PhysicsContact::isTouching () const
{
	return isValid();
}

void PhysicsContact::setEnabled (bool enabled) const {
	b2ShapeId a, b;
	fromContact(*this, a, b);
	WorldImpl* impl = nullptr;
	if (B2_IS_NON_NULL(a))
		impl = stateForWorld(worldOfBody(b2Shape_GetBody(a)));
	if (impl)
		impl->preSolveEnabled = enabled;
}

PhysicsContactEdge PhysicsContactEdge::next () const
{
	auto* node = reinterpret_cast<ContactEdgeNode*>(static_cast<uintptr_t>(_key));
	if (!node || !node->next)
		return PhysicsContactEdge();
	PhysicsContactEdge edge = PhysicsContactEdge::fromStorage(reinterpret_cast<uint64_t>(node->next));
	edge.contact = node->next->contact;
	return edge;
}

void physGetPointStates (PhysicsPointState state1[PhysicsMaxManifoldPoints],
		PhysicsPointState state2[PhysicsMaxManifoldPoints],
		const PhysicsManifold& manifold1, const PhysicsManifold& manifold2)
{
	(void)manifold1;
	(void)manifold2;
	for (int i = 0; i < PhysicsMaxManifoldPoints; ++i) {
		state1[i] = PhysicsPointState::Null;
		state2[i] = PhysicsPointState::Add;
	}
}
