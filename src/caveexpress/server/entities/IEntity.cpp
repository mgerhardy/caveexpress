#include "IEntity.h"
#include "caveexpress/server/map/Map.h"
#include "common/SpriteDefinition.h"
#include "common/ConfigManager.h"
#include "caveexpress/server/events/GameEventHandler.h"
#include <SDL_assert.h>

namespace caveexpress {

uint32_t IEntity::GLOBAL_ENTITY_NUM = 0;

IEntity::IEntity (const EntityType &type, Map& map) :
		_id(GLOBAL_ENTITY_NUM++), _type(type), _time(0), _vismask(NOTVISIBLE), _animationType(
				&Animation::NONE), _spriteAlignment(ENTITY_ALIGN_LOWER_LEFT), _state(0), _refilter(false), _touchingWater(
				false), _remove(false), _map(map), _oldGravityScale(0.0f), _animationChangeTimer(0),
				_snapshot(PhysicsVec2_zero, 0.0f, 0)
{
	setSpriteID("");
	_size = PhysicsVec2(_type.width, _type.height);

	_b2AABB.lowerBound = PhysicsVec2(FLT_MAX, FLT_MAX);
	_b2AABB.upperBound = PhysicsVec2(-FLT_MAX, -FLT_MAX);
}

IEntity::~IEntity ()
{
	deleteBodies();
}

SpriteDefPtr IEntity::getSpriteDef () const
{
	const std::string& spriteName = SpriteDefinition::get().getSpriteName(_type, getAnimationType());
	const SpriteDefPtr& def = SpriteDefinition::get().getSpriteDefinition(spriteName);
	return def;
}

void IEntity::prepareRemoval ()
{
	deleteBodies();
}

bool IEntity::isNpcFriendly() const {
	if (!isNpcCave())
		return false;
	const INPCCave *npc = assert_cast<const INPCCave*, const IEntity*>(this);
	return !npc->isDeliverPackage();
}

bool IEntity::isNpcPackage() const {
	if (!isNpcCave())
		return false;
	const INPCCave *npc = assert_cast<const INPCCave*, const IEntity*>(this);
	return npc->isDeliverPackage();
}

PhysicsVec2 IEntity::getPos () const
{
	for (BodyListConstIterator i = _bodies.begin(); i != _bodies.end(); ++i) {
		return (*i).getPosition();
	}
	return PhysicsVec2_zero;
}

float IEntity::getAngle () const
{
	for (BodyListConstIterator i = _bodies.begin(); i != _bodies.end(); ++i) {
		return (*i).getAngle();
	}
	return 0.0f;
}

void IEntity::setPos (const PhysicsVec2& pos)
{
	setPosAndAngle(pos, getAngle());
}

void IEntity::addRopeJoint (IEntity *entity)
{
	if (_ropeJoint.isValid())
		removeRopeJoint();

	PhysicsDistanceJointDef rDef;
	rDef.userData = (uintptr_t)entity;
	PhysicsBody bodyA = getBodies()[0];
	PhysicsBody bodyB = entity->getBodies()[0];

	rDef.maxLength = 0.6f;
	rDef.minLength = 0.0f;
	rDef.collideConnected = true;
	rDef.localAnchorA = PhysicsVec2_zero;
	rDef.localAnchorB = PhysicsVec2_zero;
	rDef.localAnchorA.y = getSize().y / 2.0f;
	// TODO: handle the rotation
	if (bodyA.getPosition().y < bodyB.getPosition().y) {
		rDef.localAnchorA.y *= -1.0f;
	}
	rDef.localAnchorB.y = entity->getSize().y / 2.0f;
	rDef.bodyA = bodyA;
	rDef.bodyB = bodyB;
	_ropeJoint = bodyA.getWorld()->createDistanceJoint(rDef);
	GameEvent.sendRope(getVisMask() | entity->getVisMask(), getID(), entity->getID());
}

void IEntity::removeRopeJoint ()
{
	if (!_ropeJoint.isValid())
		return;

	PhysicsBody bodyA = _ropeJoint.getBodyA();
	IEntity *entity = reinterpret_cast<IEntity*>(_ropeJoint.getUserData());
	GameEvent.removeRope(getVisMask() | entity->getVisMask(), getID());
	bodyA.getWorld()->destroyJoint(_ropeJoint);
	_ropeJoint.clear();
}

void IEntity::resetAnimationChange ()
{
	if (_animationChangeTimer != 0) {
		_map.getTimeManager().clearTimeout(_animationChangeTimer);
		_animationChangeTimer = 0;
	}
}

bool IEntity::setAnimationType (const Animation& type)
{
	if (getAnimationType() != type) {
		_animationType = &type;
		GameEvent.changeAnimation(_vismask, *this, type);
		return true;
	}
	return false;
}

void IEntity::changeAnimation (const Animation *type)
{
	_animationChangeTimer = 0;
	setAnimationType(*type);
}

int IEntity::changeAnimations (const Animation& first, const Animation& second)
{
	resetAnimationChange();
	setAnimationType(first);
	if (second.isNone())
		return 0;
	const int length = appendAnimation(second);
	return length;
}

int IEntity::appendAnimation (const Animation& type)
{
	if (type.isNone())
		return 0;
	SDL_assert(_animationChangeTimer == 0);
	const int length = SpriteDefinition::get().getAnimationLength(_type, getAnimationType());
	if (length > 0)
		_animationChangeTimer = _map.getTimeManager().setTimeout(length, this, &IEntity::changeAnimation, &type);
	else
		setAnimationType(type);
	return length;
}

void IEntity::setState (int state)
{
	Log::debug(LOG_GAMEIMPL, "entity state change to %i", state);
	_state = state;
	_refilter = true;
}

const PhysicsAABB& IEntity::getAABB () const
{
	if (!EntityTypes::isDynamic(_type))
		computeAABB();
	return _b2AABB;
}

void IEntity::update (uint32_t deltaTime)
{
	_time += deltaTime;
	for (EntityObservers::iterator i = _observers.begin(); i != _observers.end(); ++i) {
		(*i)->onUpdate(this);
	}
}

void IEntity::onContact (PhysicsContact contact, IEntity* entity)
{
	//if (EntityTypes::isLava(entity->getType())) {
	// TODO: play sound and handle "death"
	//}
}

void IEntity::endContact (PhysicsContact contact, IEntity* entity)
{
}

void IEntity::onPostSolve (PhysicsContact contact, const PhysicsContactImpulse& impulse, IEntity* entity)
{
}

void IEntity::onPreSolve (PhysicsContact contact, IEntity* entity, const PhysicsManifold& oldManifold)
{
}

bool IEntity::shouldCollide (const IEntity* entity) const
{
	return entity->isSolid();
}

bool IEntity::shouldRefilter () const
{
	return _refilter;
}

void IEntity::remove ()
{
	deleteBodies();
}

bool IEntity::shouldApplyWind () const
{
	return false;
}

void IEntity::snapshot () const
{
	_snapshot.pos = getPos();
	_snapshot.angle = getAngle();
	_snapshot.state = getState();
}

inline bool IEntity::isSnapshotDirty () const
{
	if (_snapshot.count <= 0) {
		++_snapshot.count;
		return true;
	}

	if (_snapshot.state != getState())
		return true;
	if (!fequals(_snapshot.angle, getAngle(), 0.2f))
		return true;
	if (!physVec2Equals(_snapshot.pos, getPos(), 0.011f))
		return true;

	_snapshot.count = 0;
	return false;
}

bool IEntity::isDirty () const
{
	return !isRemove() && isDynamic() && isSnapshotDirty();
}

bool IEntity::isRemove () const
{
	return _remove;
}

void IEntity::onSpawn ()
{
}

float IEntity::getDensity () const
{
	float density = 0.0f;
	for (BodyListConstIterator i = _bodies.begin(); i != _bodies.end(); ++i) {
		for (PhysicsFixture f = (*i).getFixtureList(); f.isValid(); f = f.getNext()) {
			density += f.getDensity();
		}
	}
	return density /= (float) _bodies.size();
}

float IEntity::getGravityScale () const
{
	float gravityScale = 0.0f;
	for (BodyListConstIterator i = _bodies.begin(); i != _bodies.end(); ++i) {
		gravityScale += (*i).getGravityScale();
	}
	return gravityScale /= (float) _bodies.size();
}

float IEntity::getMass () const
{
	float mass = 0.0f;
	for (BodyListConstIterator i = _bodies.begin(); i != _bodies.end(); ++i) {
		mass += (*i).getMass();
	}
	return mass;
}

PhysicsVec2 IEntity::getGravity () const
{
	if (_bodies.empty())
		return PhysicsVec2_zero;

	return (*_bodies.begin()).getWorld()->getGravity();
}

void IEntity::computeAABB () const
{
	_b2AABB.lowerBound = PhysicsVec2(FLT_MAX, FLT_MAX);
	_b2AABB.upperBound = PhysicsVec2(-FLT_MAX, -FLT_MAX);

	if (_bodies.empty()) {
		const PhysicsVec2& pos = getPos();
		const PhysicsVec2& size = getSize();
		const float halfWidth = size.x / 2.0f;
		const float halfHeight = size.y / 2.0f;
		_b2AABB.lowerBound.x = pos.x - halfWidth;
		_b2AABB.lowerBound.y = pos.y - halfHeight;
		_b2AABB.upperBound.x = pos.x + halfWidth;
		_b2AABB.upperBound.y = pos.y + halfHeight;
		return;
	}

	for (BodyListConstIterator i = _bodies.begin(); i != _bodies.end(); ++i) {
		PhysicsAABB aabb;
		PhysicsTransform t;
		t.setIdentity();
		for (PhysicsFixture fixture = (*i).getFixtureList(); fixture.isValid(); fixture = fixture.getNext()) {
			const int childCount = fixture.getChildCount();
			for (int child = 0; child < childCount; ++child) {
				fixture.computeAABB(aabb, t, child);
				_b2AABB.combine(aabb);
			}
		}
	}
}

inline void IEntity::deleteBodies ()
{
	PhysicsWorld *world = getMap().getWorld();
	for (BodyListIterator i = _bodies.begin(); i != _bodies.end(); ++i) {
		world->destroyBody(*i);
		(*i).clear();
	}
	_bodies.clear();
}

bool IEntity::isVisibleFor (const IEntity* entity) const
{
	if (_bodies.empty() || isRemove())
		return false;

#if 0
	const PhysicsVec2 &selfPos = getPos();
	const PhysicsVec2 &otherPos = entity->getPos();
	// TODO: this is very silly - improve the checks
	if (physAbs(selfPos.x - otherPos.x) > Config.getScreenMapWidth())
		return false;
	if (physAbs(selfPos.y - otherPos.y) > Config.getScreenMapHeight())
		return false;
#endif
	return true;
}

Direction IEntity::getVelocityDirection (IEntity* entity, float vEpsilon) const
{
	Direction dir = 0;
	const PhysicsVec2& velocity = entity->getLinearVelocity();
	if (velocity.y > vEpsilon) {
		dir |= DIRECTION_DOWN;
	} else if (velocity.y < vEpsilon) {
		dir |= DIRECTION_UP;
	}

	if (velocity.x > vEpsilon) {
		dir |= DIRECTION_RIGHT;
	} else if (velocity.x < vEpsilon) {
		dir |= DIRECTION_LEFT;
	}

	return dir;
}

PhysicsVec2 IEntity::getImpactVelocity (PhysicsContact contact) const
{
	const PhysicsBody bodyA = contact.getFixtureA().getBody();
	const PhysicsBody bodyB = contact.getFixtureB().getBody();
	PhysicsWorldManifold worldManifold;
	contact.getWorldManifold(worldManifold);
	const PhysicsVec2 vel1 = bodyA.getLinearVelocityFromWorldPoint(worldManifold.points[0]);
	const PhysicsVec2 vel2 = bodyB.getLinearVelocityFromWorldPoint(worldManifold.points[0]);
	const PhysicsVec2 impactVelocity = vel1 - vel2;
	return impactVelocity;
}

}
