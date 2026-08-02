#include "physics/box2d24/Native.h"
#include "physics/PhysicsCallbacks.h"
#include <vector>

using namespace phys24;

namespace {

class ContactListenerAdapter: public b2ContactListener {
public:
	IPhysicsContactListener* listener = nullptr;

	void BeginContact (b2Contact* contact) override
	{
		if (listener)
			listener->beginContact(toContact(contact));
	}

	void EndContact (b2Contact* contact) override
	{
		if (listener)
			listener->endContact(toContact(contact));
	}

	void PreSolve (b2Contact* contact, const b2Manifold* oldManifold) override
	{
		if (listener)
			listener->preSolve(toContact(contact), toManifold(oldManifold));
	}

	void PostSolve (b2Contact* contact, const b2ContactImpulse* impulse) override
	{
		if (!listener)
			return;
		PhysicsContactImpulse out;
		out.count = impulse ? impulse->count : 0;
		for (int i = 0; i < out.count && i < PhysicsMaxManifoldPoints; ++i) {
			out.normalImpulses[i] = impulse->normalImpulses[i];
			out.tangentImpulses[i] = impulse->tangentImpulses[i];
		}
		listener->postSolve(toContact(contact), out);
	}
};

class ContactFilterAdapter: public b2ContactFilter {
public:
	IPhysicsContactFilter* filter = nullptr;

	bool ShouldCollide (b2Fixture* fixtureA, b2Fixture* fixtureB) override
	{
		if (!filter)
			return b2ContactFilter::ShouldCollide(fixtureA, fixtureB);
		return filter->shouldCollide(toFixture(fixtureA), toFixture(fixtureB));
	}
};

class DestructionListenerAdapter: public b2DestructionListener {
public:
	IPhysicsDestructionListener* listener = nullptr;

	void SayGoodbye (b2Joint* joint) override
	{
		if (listener)
			listener->sayGoodbye(toJoint(joint));
	}

	void SayGoodbye (b2Fixture* fixture) override
	{
		if (listener)
			listener->sayGoodbye(toFixture(fixture));
	}
};

class DebugDrawAdapter: public b2Draw {
public:
	IPhysicsDebugDraw* draw = nullptr;

	void DrawPolygon (const b2Vec2* vertices, int32 vertexCount, const b2Color& color) override
	{
		if (!draw)
			return;
		PhysicsVec2 tmp[b2_maxPolygonVertices];
		const int n = vertexCount < b2_maxPolygonVertices ? vertexCount : b2_maxPolygonVertices;
		for (int i = 0; i < n; ++i)
			tmp[i] = toVec(vertices[i]);
		draw->drawPolygon(tmp, n, PhysicsColor(color.r, color.g, color.b, color.a));
	}

	void DrawSolidPolygon (const b2Vec2* vertices, int32 vertexCount, const b2Color& color) override
	{
		if (!draw)
			return;
		PhysicsVec2 tmp[b2_maxPolygonVertices];
		const int n = vertexCount < b2_maxPolygonVertices ? vertexCount : b2_maxPolygonVertices;
		for (int i = 0; i < n; ++i)
			tmp[i] = toVec(vertices[i]);
		draw->drawSolidPolygon(tmp, n, PhysicsColor(color.r, color.g, color.b, color.a));
	}

	void DrawCircle (const b2Vec2& center, float radius, const b2Color& color) override
	{
		if (draw)
			draw->drawCircle(toVec(center), radius, PhysicsColor(color.r, color.g, color.b, color.a));
	}

	void DrawSolidCircle (const b2Vec2& center, float radius, const b2Vec2& axis, const b2Color& color) override
	{
		if (draw)
			draw->drawSolidCircle(toVec(center), radius, toVec(axis), PhysicsColor(color.r, color.g, color.b, color.a));
	}

	void DrawSegment (const b2Vec2& p1, const b2Vec2& p2, const b2Color& color) override
	{
		if (draw)
			draw->drawSegment(toVec(p1), toVec(p2), PhysicsColor(color.r, color.g, color.b, color.a));
	}

	void DrawTransform (const b2Transform& xf) override
	{
		if (!draw)
			return;
		PhysicsTransform t;
		t.p = toVec(xf.p);
		t.c = xf.q.c;
		t.s = xf.q.s;
		draw->drawTransform(t);
	}

	void DrawPoint (const b2Vec2& p, float size, const b2Color& color) override
	{
		if (draw)
			draw->drawPoint(toVec(p), size, PhysicsColor(color.r, color.g, color.b, color.a));
	}
};

class RayCastAdapter: public b2RayCastCallback {
public:
	IPhysicsRayCastCallback* callback = nullptr;

	float ReportFixture (b2Fixture* fixture, const b2Vec2& point, const b2Vec2& normal, float fraction) override
	{
		if (!callback)
			return 0.0f;
		return callback->reportFixture(toFixture(fixture), toVec(point), toVec(normal), fraction);
	}
};

struct WorldImpl {
	b2World world;
	WorldState state;
	ContactListenerAdapter contactListenerAdapter;
	ContactFilterAdapter contactFilterAdapter;
	DestructionListenerAdapter destructionListenerAdapter;
	DebugDrawAdapter debugDrawAdapter;

	explicit WorldImpl (const b2Vec2& gravity) : world(gravity)
	{
		state.world = &world;
		worldMap()[&world] = &state;
	}

	~WorldImpl ()
	{
		worldMap().erase(&world);
	}
};

inline WorldImpl* implOf (void* p)
{
	return static_cast<WorldImpl*>(p);
}

} // namespace

// ---------------------------------------------------------------------------
// PhysicsWorld
// ---------------------------------------------------------------------------

PhysicsWorld::PhysicsWorld (const PhysicsVec2& gravity)
{
	auto* impl = new WorldImpl(toB2(gravity));
	impl->state.owner = this;
	_impl = impl;
}

PhysicsWorld::~PhysicsWorld ()
{
	delete implOf(_impl);
	_impl = nullptr;
}

void PhysicsWorld::step (float timeStep, int velocityIterations, int positionIterations)
{
	implOf(_impl)->world.Step(timeStep, velocityIterations, positionIterations);
}

void PhysicsWorld::step (float timeStep, int subStepCount)
{
	// 2.4 ignores sub-steps; map to a reasonable iteration pair.
	(void)subStepCount;
	step(timeStep, 8, 3);
}

PhysicsVec2 PhysicsWorld::getGravity () const
{
	return toVec(implOf(_impl)->world.GetGravity());
}

void PhysicsWorld::setGravity (const PhysicsVec2& gravity)
{
	implOf(_impl)->world.SetGravity(toB2(gravity));
}

void PhysicsWorld::setAutoClearForces (bool flag)
{
	implOf(_impl)->world.SetAutoClearForces(flag);
}

PhysicsBody PhysicsWorld::createBody (const PhysicsBodyDef& def)
{
	b2BodyDef bd;
	bd.type = toB2(def.type);
	bd.position = toB2(def.position);
	bd.angle = def.angle;
	bd.linearVelocity = toB2(def.linearVelocity);
	bd.angularVelocity = def.angularVelocity;
	bd.linearDamping = def.linearDamping;
	bd.angularDamping = def.angularDamping;
	bd.fixedRotation = def.fixedRotation;
	bd.bullet = def.bullet;
	bd.gravityScale = def.gravityScale;
	bd.userData.pointer = def.userData;
	return toBody(implOf(_impl)->world.CreateBody(&bd));
}

void PhysicsWorld::destroyBody (PhysicsBody body)
{
	b2Body* b = toB2(body);
	if (b)
		implOf(_impl)->world.DestroyBody(b);
}

PhysicsJoint PhysicsWorld::createDistanceJoint (const PhysicsDistanceJointDef& def)
{
	b2DistanceJointDef jd;
	if (def.useWorldAnchors) {
		jd.Initialize(toB2(def.bodyA), toB2(def.bodyB), toB2(def.worldAnchorA), toB2(def.worldAnchorB));
	} else {
		jd.bodyA = toB2(def.bodyA);
		jd.bodyB = toB2(def.bodyB);
		jd.localAnchorA = toB2(def.localAnchorA);
		jd.localAnchorB = toB2(def.localAnchorB);
	}
	jd.minLength = def.minLength;
	jd.maxLength = def.maxLength;
	jd.collideConnected = def.collideConnected;
	jd.userData.pointer = def.userData;
	return toJoint(implOf(_impl)->world.CreateJoint(&jd));
}

PhysicsJoint PhysicsWorld::createRevoluteJoint (const PhysicsRevoluteJointDef& def)
{
	b2RevoluteJointDef jd;
	if (def.useWorldPivot) {
		jd.Initialize(toB2(def.bodyA), toB2(def.bodyB), toB2(def.worldPivot));
	} else {
		jd.bodyA = toB2(def.bodyA);
		jd.bodyB = toB2(def.bodyB);
		jd.localAnchorA = toB2(def.localAnchorA);
		jd.localAnchorB = toB2(def.localAnchorB);
	}
	jd.lowerAngle = def.lowerAngle;
	jd.upperAngle = def.upperAngle;
	jd.enableLimit = def.enableLimit;
	jd.enableMotor = def.enableMotor;
	jd.motorSpeed = def.motorSpeed;
	jd.maxMotorTorque = def.maxMotorTorque;
	jd.collideConnected = def.collideConnected;
	jd.userData.pointer = def.userData;
	return toJoint(implOf(_impl)->world.CreateJoint(&jd));
}

void PhysicsWorld::destroyJoint (PhysicsJoint joint)
{
	b2Joint* j = toB2(joint);
	if (j)
		implOf(_impl)->world.DestroyJoint(j);
}

void PhysicsWorld::rayCast (IPhysicsRayCastCallback& callback, const PhysicsVec2& point1, const PhysicsVec2& point2) const
{
	RayCastAdapter adapter;
	adapter.callback = &callback;
	implOf(_impl)->world.RayCast(&adapter, toB2(point1), toB2(point2));
}

void PhysicsWorld::setContactListener (IPhysicsContactListener* listener)
{
	WorldImpl* impl = implOf(_impl);
	impl->state.contactListener = listener;
	impl->contactListenerAdapter.listener = listener;
	impl->world.SetContactListener(listener ? &impl->contactListenerAdapter : nullptr);
}

void PhysicsWorld::setContactFilter (IPhysicsContactFilter* filter)
{
	WorldImpl* impl = implOf(_impl);
	impl->state.contactFilter = filter;
	impl->contactFilterAdapter.filter = filter;
	impl->world.SetContactFilter(filter ? &impl->contactFilterAdapter : nullptr);
}

void PhysicsWorld::setDestructionListener (IPhysicsDestructionListener* listener)
{
	WorldImpl* impl = implOf(_impl);
	impl->state.destructionListener = listener;
	impl->destructionListenerAdapter.listener = listener;
	impl->world.SetDestructionListener(listener ? &impl->destructionListenerAdapter : nullptr);
}

void PhysicsWorld::setDebugDraw (IPhysicsDebugDraw* draw, uint32_t flags)
{
	WorldImpl* impl = implOf(_impl);
	impl->state.debugDraw = draw;
	impl->debugDrawAdapter.draw = draw;
	if (draw) {
		uint32 b2flags = 0;
		if (flags & PhysicsDrawShapes)
			b2flags |= b2Draw::e_shapeBit;
		if (flags & PhysicsDrawJoints)
			b2flags |= b2Draw::e_jointBit;
		if (flags & PhysicsDrawAabb)
			b2flags |= b2Draw::e_aabbBit;
		if (flags & PhysicsDrawPairs)
			b2flags |= b2Draw::e_pairBit;
		if (flags & PhysicsDrawCenterOfMass)
			b2flags |= b2Draw::e_centerOfMassBit;
		if (b2flags == 0) {
			b2flags = b2Draw::e_shapeBit | b2Draw::e_jointBit | b2Draw::e_aabbBit
					| b2Draw::e_pairBit | b2Draw::e_centerOfMassBit;
		}
		impl->debugDrawAdapter.SetFlags(b2flags);
		impl->world.SetDebugDraw(&impl->debugDrawAdapter);
	} else {
		impl->world.SetDebugDraw(nullptr);
	}
}

void PhysicsWorld::debugDraw () const
{
	implOf(_impl)->world.DebugDraw();
}

void PhysicsWorld::dump () const
{
	implOf(_impl)->world.Dump();
}

PhysicsWorld* PhysicsWorld::fromBody (PhysicsBody body)
{
	b2Body* b = toB2(body);
	if (!b)
		return nullptr;
	WorldState* st = stateFor(b->GetWorld());
	return st ? st->owner : nullptr;
}

// ---------------------------------------------------------------------------
// PhysicsBody
// ---------------------------------------------------------------------------

PhysicsVec2 PhysicsBody::getPosition () const
{
	return toVec(toB2(*this)->GetPosition());
}

float PhysicsBody::getAngle () const
{
	return toB2(*this)->GetAngle();
}

PhysicsVec2 PhysicsBody::getWorldCenter () const
{
	return toVec(toB2(*this)->GetWorldCenter());
}

PhysicsVec2 PhysicsBody::getWorldPoint (const PhysicsVec2& localPoint) const
{
	return toVec(toB2(*this)->GetWorldPoint(toB2(localPoint)));
}

PhysicsVec2 PhysicsBody::getLinearVelocity () const
{
	return toVec(toB2(*this)->GetLinearVelocity());
}

float PhysicsBody::getAngularVelocity () const
{
	return toB2(*this)->GetAngularVelocity();
}

float PhysicsBody::getInertia () const
{
	return toB2(*this)->GetInertia();
}

float PhysicsBody::getMass () const
{
	return toB2(*this)->GetMass();
}

float PhysicsBody::getGravityScale () const
{
	return toB2(*this)->GetGravityScale();
}

PhysicsVec2 PhysicsBody::getLinearVelocityFromWorldPoint (const PhysicsVec2& worldPoint) const
{
	return toVec(toB2(*this)->GetLinearVelocityFromWorldPoint(toB2(worldPoint)));
}

PhysicsUserData PhysicsBody::getUserData () const
{
	return toB2(*this)->GetUserData().pointer;
}

PhysicsWorld* PhysicsBody::getWorld () const
{
	return PhysicsWorld::fromBody(*this);
}

void PhysicsBody::setTransform (const PhysicsVec2& position, float angle) const {
	toB2(*this)->SetTransform(toB2(position), angle);
}

void PhysicsBody::setLinearVelocity (const PhysicsVec2& v) const {
	toB2(*this)->SetLinearVelocity(toB2(v));
}

void PhysicsBody::setAngularVelocity (float omega) const {
	toB2(*this)->SetAngularVelocity(omega);
}

void PhysicsBody::setGravityScale (float scale) const {
	toB2(*this)->SetGravityScale(scale);
}

void PhysicsBody::setLinearDamping (float damping) const {
	toB2(*this)->SetLinearDamping(damping);
}

void PhysicsBody::setAngularDamping (float damping) const {
	toB2(*this)->SetAngularDamping(damping);
}

void PhysicsBody::setFixedRotation (bool flag) const {
	toB2(*this)->SetFixedRotation(flag);
}

void PhysicsBody::setEnabled (bool flag) const {
	toB2(*this)->SetEnabled(flag);
}

void PhysicsBody::setUserData (PhysicsUserData data) const {
	b2BodyUserData ud;
	ud.pointer = data;
	toB2(*this)->GetUserData() = ud;
}

void PhysicsBody::applyForce (const PhysicsVec2& force, const PhysicsVec2& point, bool wake) const {
	toB2(*this)->ApplyForce(toB2(force), toB2(point), wake);
}

void PhysicsBody::applyForceToCenter (const PhysicsVec2& force, bool wake) const {
	toB2(*this)->ApplyForceToCenter(toB2(force), wake);
}

void PhysicsBody::applyLinearImpulse (const PhysicsVec2& impulse, const PhysicsVec2& point, bool wake) const {
	toB2(*this)->ApplyLinearImpulse(toB2(impulse), toB2(point), wake);
}

void PhysicsBody::applyTorque (float torque, bool wake) const {
	toB2(*this)->ApplyTorque(torque, wake);
}

PhysicsFixture PhysicsBody::createFixture (const PhysicsFixtureDef& def) const {
	b2FixtureDef fd;
	fd.density = def.density;
	fd.friction = def.friction;
	fd.restitution = def.restitution;
	fd.isSensor = def.isSensor;
	fd.filter = toB2(def.filter);
	fd.userData.pointer = def.userData;

	b2PolygonShape poly;
	b2CircleShape circle;
	b2EdgeShape edge;

	switch (def.shapeType) {
	case PhysicsShapeType::Circle:
		circle.m_p = toB2(def.circleCenter);
		circle.m_radius = def.radius;
		fd.shape = &circle;
		break;
	case PhysicsShapeType::Edge:
		edge.SetTwoSided(toB2(def.vertices[0]), toB2(def.vertices[1]));
		fd.shape = &edge;
		break;
	case PhysicsShapeType::Polygon:
	default:
		if (def.useBox) {
			poly.SetAsBox(def.boxHalfWidth, def.boxHalfHeight);
		} else {
			b2Vec2 verts[b2_maxPolygonVertices];
			const int n = def.vertexCount < b2_maxPolygonVertices ? def.vertexCount : b2_maxPolygonVertices;
			for (int i = 0; i < n; ++i)
				verts[i] = toB2(def.vertices[i]);
			poly.Set(verts, n);
		}
		fd.shape = &poly;
		break;
	}

	return toFixture(toB2(*this)->CreateFixture(&fd));
}

PhysicsFixture PhysicsBody::getFixtureList () const
{
	return toFixture(toB2(*this)->GetFixtureList());
}

PhysicsContactEdge PhysicsBody::getContactList () const
{
	b2ContactEdge* edge = toB2(*this)->GetContactList();
	if (!edge)
		return PhysicsContactEdge();
	PhysicsContactEdge out = PhysicsContactEdge::fromStorage(reinterpret_cast<uint64_t>(edge));
	out.contact = toContact(edge->contact);
	return out;
}

// ---------------------------------------------------------------------------
// PhysicsFixture
// ---------------------------------------------------------------------------

PhysicsBody PhysicsFixture::getBody () const
{
	return toBody(toB2(*this)->GetBody());
}

PhysicsShapeType PhysicsFixture::getShapeType () const
{
	return toShapeType(toB2(*this)->GetShape()->GetType());
}

float PhysicsFixture::getDensity () const
{
	return toB2(*this)->GetDensity();
}

PhysicsFilter PhysicsFixture::getFilterData () const
{
	return toFilter(toB2(*this)->GetFilterData());
}

PhysicsUserData PhysicsFixture::getUserData () const
{
	return toB2(*this)->GetUserData().pointer;
}

void PhysicsFixture::setUserData (PhysicsUserData data) const {
	b2FixtureUserData ud;
	ud.pointer = data;
	toB2(*this)->GetUserData() = ud;
}

void PhysicsFixture::refilter () const {
	toB2(*this)->Refilter();
}

PhysicsFixture PhysicsFixture::getNext () const
{
	return toFixture(toB2(*this)->GetNext());
}

int PhysicsFixture::getPolygonVertexCount () const
{
	const b2Shape* shape = toB2(*this)->GetShape();
	if (shape->GetType() != b2Shape::e_polygon)
		return 0;
	return static_cast<const b2PolygonShape*>(shape)->m_count;
}

PhysicsVec2 PhysicsFixture::getPolygonVertex (int index) const
{
	const auto* poly = static_cast<const b2PolygonShape*>(toB2(*this)->GetShape());
	return toVec(poly->m_vertices[index]);
}

float PhysicsFixture::getCircleRadius () const
{
	const b2Shape* shape = toB2(*this)->GetShape();
	if (shape->GetType() != b2Shape::e_circle)
		return 0.0f;
	return shape->m_radius;
}

PhysicsVec2 PhysicsFixture::getCircleLocalCenter () const
{
	const b2Shape* shape = toB2(*this)->GetShape();
	if (shape->GetType() != b2Shape::e_circle)
		return PhysicsVec2_zero;
	return toVec(static_cast<const b2CircleShape*>(shape)->m_p);
}

int PhysicsFixture::getChildCount () const
{
	return toB2(*this)->GetShape()->GetChildCount();
}

void PhysicsFixture::computeAABB (PhysicsAABB& aabb, const PhysicsTransform& xf, int childIndex) const
{
	b2Transform t;
	t.p = toB2(xf.p);
	t.q.Set(xf.s == 0.0f && xf.c == 1.0f ? 0.0f : atan2f(xf.s, xf.c));
	// Prefer setting c/s directly if available
	t.q.c = xf.c;
	t.q.s = xf.s;
	b2AABB box;
	toB2(*this)->GetShape()->ComputeAABB(&box, t, childIndex);
	aabb.lowerBound = toVec(box.lowerBound);
	aabb.upperBound = toVec(box.upperBound);
}

// ---------------------------------------------------------------------------
// PhysicsJoint
// ---------------------------------------------------------------------------

PhysicsBody PhysicsJoint::getBodyA () const
{
	return toBody(toB2(*this)->GetBodyA());
}

PhysicsBody PhysicsJoint::getBodyB () const
{
	return toBody(toB2(*this)->GetBodyB());
}

PhysicsUserData PhysicsJoint::getUserData () const
{
	return toB2(*this)->GetUserData().pointer;
}

void PhysicsJoint::setUserData (PhysicsUserData data) const {
	b2JointUserData ud;
	ud.pointer = data;
	toB2(*this)->GetUserData() = ud;
}

float PhysicsJoint::getLength () const
{
	return static_cast<b2DistanceJoint*>(toB2(*this))->GetLength();
}

void PhysicsJoint::setLength (float length) const {
	static_cast<b2DistanceJoint*>(toB2(*this))->SetLength(length);
}

void PhysicsJoint::setMotorSpeed (float speed) const {
	static_cast<b2RevoluteJoint*>(toB2(*this))->SetMotorSpeed(speed);
}

// ---------------------------------------------------------------------------
// PhysicsContact
// ---------------------------------------------------------------------------

PhysicsFixture PhysicsContact::getFixtureA () const
{
	return toFixture(toB2(*this)->GetFixtureA());
}

PhysicsFixture PhysicsContact::getFixtureB () const
{
	return toFixture(toB2(*this)->GetFixtureB());
}

PhysicsManifold PhysicsContact::getManifold () const
{
	return toManifold(toB2(*this)->GetManifold());
}

void PhysicsContact::getWorldManifold (PhysicsWorldManifold& manifold) const
{
	b2WorldManifold wm;
	toB2(*this)->GetWorldManifold(&wm);
	manifold.normal = toVec(wm.normal);
	for (int i = 0; i < PhysicsMaxManifoldPoints; ++i)
		manifold.points[i] = toVec(wm.points[i]);
}

bool PhysicsContact::isTouching () const
{
	return toB2(*this)->IsTouching();
}

void PhysicsContact::setEnabled (bool enabled) const {
	toB2(*this)->SetEnabled(enabled);
}

PhysicsContactEdge PhysicsContactEdge::next () const
{
	auto* edge = reinterpret_cast<b2ContactEdge*>(static_cast<uintptr_t>(_key));
	if (!edge || !edge->next)
		return PhysicsContactEdge();
	PhysicsContactEdge out = PhysicsContactEdge::fromStorage(reinterpret_cast<uint64_t>(edge->next));
	out.contact = toContact(edge->next->contact);
	return out;
}

void physGetPointStates (PhysicsPointState state1[PhysicsMaxManifoldPoints],
		PhysicsPointState state2[PhysicsMaxManifoldPoints],
		const PhysicsManifold& manifold1, const PhysicsManifold& manifold2)
{
	b2Manifold m1{};
	b2Manifold m2{};
	m1.pointCount = manifold1.pointCount;
	m2.pointCount = manifold2.pointCount;
	for (int i = 0; i < manifold1.pointCount; ++i) {
		m1.points[i].normalImpulse = manifold1.points[i].normalImpulse;
		m1.points[i].tangentImpulse = manifold1.points[i].tangentImpulse;
	}
	for (int i = 0; i < manifold2.pointCount; ++i) {
		m2.points[i].normalImpulse = manifold2.points[i].normalImpulse;
		m2.points[i].tangentImpulse = manifold2.points[i].tangentImpulse;
	}
	b2PointState s1[b2_maxManifoldPoints];
	b2PointState s2[b2_maxManifoldPoints];
	b2GetPointStates(s1, s2, &m1, &m2);
	for (int i = 0; i < PhysicsMaxManifoldPoints; ++i) {
		state1[i] = toPointState(s1[i]);
		state2[i] = toPointState(s2[i]);
	}
}
