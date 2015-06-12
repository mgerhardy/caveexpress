#pragma once

#include <Box2D/Box2D.h>
#include "common/Logger.h"
#include "common/Timer.h"
#include "common/IMap.h"
#include "caveexpress/shared/CaveExpressEntityType.h"
#include "caveexpress/shared/CaveExpressAnimation.h"
#include "common/Direction.h"
#include "common/EntityAlignment.h"
#include "common/EntityState.h"
#include "common/EntityType.h"
#include "common/Pointers.h"
#include "common/Math.h"
#include <vector>
#include <list>
#include <string>
#include <sstream>
#include <stdint.h>

typedef int32_t VisMask;
#define NOTVISIBLE 0x80000000

class SpriteDef;
typedef SharedPtr<SpriteDef> SpriteDefPtr;

namespace caveexpress {

class Map;
class IEntity;

class IEntityObserver {
public:
	virtual ~IEntityObserver ()
	{
	}

	// callback that is called each time the update method of
	// a physics entity is called
	virtual void onUpdate (IEntity *entity)
	{
	}
};

class IEntity {
protected:
	std::vector<std::string> _spriteIDs;
	uint16_t _id;
	const EntityType &_type;
	uint32_t _time;
	static uint32_t GLOBAL_ENTITY_NUM;
	// bitmask of client ids that can see this entity
	VisMask _vismask;
	const Animation *_animationType;
	EntityAlignment _spriteAlignment;
	// the current entity state
	int _state;
	// this is the server size of the object
	b2Vec2 _size;

	typedef std::vector<IEntityObserver*> EntityObservers;
	EntityObservers _observers;

	typedef std::vector<b2Body*> BodyList;
	typedef BodyList::iterator BodyListIterator;
	typedef BodyList::const_iterator BodyListConstIterator;
	BodyList _bodies;
	// if a state change was made we have to reset the physic collide filters
	bool _refilter;
	bool _touchingWater;
	mutable b2AABB _b2AABB;
	b2RopeJoint* _ropeJoint;

	// if the entity should be removed, set this to true
	bool _remove;

	Map& _map;

	float _oldGravityScale;

private:
	TimerID _animationChangeTimer;

	void changeAnimation (const Animation *);
	void computeAABB () const;
	void deleteBodies ();

	bool isSnapshotDirty () const;

	struct Snapshot {
		b2Vec2 pos;
		float angle;
		int state;
		uint8_t count;

		Snapshot (const b2Vec2& __pos, float __angle, int __state) :
				pos(__pos), angle(__angle), state(__state), count(0)
		{
		}
	};

	mutable Snapshot _snapshot;
public:
	IEntity (const EntityType &type, Map& map);

	virtual ~IEntity ();

	virtual void snapshot () const;

	// allows us to clean all box2d stuff before the dtors are called
	// this is useful in some situations where the bodies of the entity
	// are e.g. still tied to some existing contact
	virtual void prepareRemoval ();

	// returns true if the entity should get removed
	// we may not remove it in the tick due to concurrent modification
	// exceptions while looping over the entity lists
	virtual bool isRemove () const;

	// called when the entity is added to the world
	virtual void onSpawn ();

	// returns the axis-aligned-bounding-box
	virtual const b2AABB& getAABB () const;

	// returns the center of the object
	// this is not the screen coordinate!
	virtual const b2Vec2& getPos () const;

	// returns the angle of the entity in radians
	virtual float getAngle () const;

	virtual void setPos (const b2Vec2& pos);

	virtual void setState (int state);

	virtual void update (uint32_t deltaTime);

	// notifies the entity that another entity was hit. The hit entity will get
	// notified about the contact, too. The order in which the listeners are called
	// is not defined
	virtual void onContact (b2Contact* contact, IEntity* entity);

	// notifies the entity that the contact to another entity has ended
	virtual void endContact (b2Contact* contact, IEntity* entity);

	// This lets you inspect a contact after the solver is finished. This is useful
	// for inspecting impulses. This is only called for contacts that are touching,
	// solid, and awake.
	virtual void onPostSolve (b2Contact* contact, const b2ContactImpulse* impulse, IEntity* entity);

	/// This is called after a contact is updated. This allows you to inspect a
	/// contact before it goes to the solver. If you are careful, you can modify the
	/// contact manifold (e.g. disable contact).
	/// This is called after onContact is called.
	/// This is not called for sensor bodies.
	virtual void onPreSolve (b2Contact* contact, IEntity* entity, const b2Manifold* oldManifold);

	// return false if the entity should not collide with the given entity. true
	// if it should collide
	virtual bool shouldCollide (const IEntity* entity) const;

	// return true if the should collide callback should be reevaluated in the next frame
	virtual bool shouldRefilter () const;

	virtual void remove ();

	virtual bool shouldApplyWind () const;

	// returns true if something has changed that the clients have to know about
	virtual bool isDirty () const;

	void addRopeJoint (IEntity *entity);
	void removeRopeJoint ();

	virtual SpriteDefPtr getSpriteDef () const;

	void resetAnimationChange ();
	bool setAnimationType (const Animation& type);
	// returns the overall needed time until both animations are completely played
	int changeAnimations (const Animation& first, const Animation& second = Animation::NONE);
	// returns the milliseconds that have to pass until the appended animation is played
	int appendAnimation (const Animation& type);

	bool isVisibleFor (const IEntity* entity) const;

	virtual float getDensity () const;
	float getGravityScale () const;
	float getMass () const;
	b2Vec2 getGravity () const;

	inline uint16_t getID () const
	{
		return _id;
	}

	inline const std::string& getSpriteID () const
	{
		return _spriteIDs[0];
	}

	inline void setSpriteID (const std::string& spriteID)
	{
		_spriteIDs.clear();
		addSpriteID(spriteID);
	}

	inline void addSpriteID (const std::string& spriteID)
	{
		_spriteIDs.push_back(spriteID);
	}

	inline const b2Vec2& getSize () const
	{
		return _size;
	}

	inline Map& getMap()
	{
		return _map;
	}

	// register an observer for this entity
	void registerObserver (IEntityObserver *observer)
	{
		_observers.push_back(observer);
	}

	// remove a previously registered observer from this entity
	void removeObserver (IEntityObserver *observer)
	{
		// Traverse through the list and try to find the specified observer
		for (EntityObservers::iterator i = _observers.begin(); i != _observers.end();
		/* in-loop increment */) {
			if (*i == observer) {
				_observers.erase(i++);
				break;
			} else {
				++i;
			}
		}
	}

	// returns the type of the entity
	inline const EntityType& getType () const
	{
		return _type;
	}

	inline bool isDestroyed () const
	{
		return getState() == EntityState::ENTITY_DESTROYED;
	}

	inline bool isNpc () const
	{
		return EntityTypes::isNpc(_type);
	}

	inline bool isPlayer () const
	{
		return EntityTypes::isPlayer(_type);
	}

	// true for entities that must be updated in the client (e.g. no fixed position)
	inline bool isDynamic () const
	{
		return EntityTypes::isDynamic(_type);
	}

	inline bool isCave () const
	{
		return EntityTypes::isCave(_type);
	}

	inline bool isCollectable () const
	{
		return EntityTypes::isCollectable(_type);
	}

	inline bool isApple () const
	{
		return EntityTypes::isApple(_type);
	}

	inline bool isBanana () const
	{
		return EntityTypes::isBanana(_type);
	}

	inline bool isFruit () const
	{
		return EntityTypes::isFruit(_type);
	}

	inline bool isBomb () const
	{
		return EntityTypes::isBomb(_type);
	}

	inline bool isStone () const
	{
		return EntityTypes::isStone(_type);
	}

	inline bool isPackageTarget () const
	{
		return EntityTypes::isPackageTarget(_type);
	}

	inline bool isPackage () const
	{
		return EntityTypes::isPackage(_type);
	}

	inline bool isNpcCave () const
	{
		return EntityTypes::isNpcCave(_type);
	}

	bool isNpcFriendly () const;

	bool isNpcPackage () const;

	inline bool isNpcFlying () const
	{
		return EntityTypes::isNpcFlying(_type);
	}

	inline bool isNpcAggressive () const
	{
		return EntityTypes::isNpcAggressive(_type);
	}

	inline bool isNpcBlowing () const
	{
		return EntityTypes::isNpcBlowing(_type);
	}

	inline bool isNpcFish () const
	{
		return EntityTypes::isNpcFish(_type);
	}

	inline bool isNpcWalking () const
	{
		return EntityTypes::isNpcWalking(_type);
	}

	inline bool isEgg () const
	{
		return EntityTypes::isEgg(_type);
	}

	inline bool isNpcMammut () const
	{
		return EntityTypes::isNpcMammut(_type);
	}

	inline bool isNpcAttacking () const
	{
		return EntityTypes::isNpcAttacking(_type);
	}

	inline bool isTree () const
	{
		return EntityTypes::isTree(_type);
	}

	inline bool isGround () const
	{
		return EntityTypes::isGround(_type);
	}

	inline bool isWindow () const
	{
		return EntityTypes::isWindow(_type);
	}

	inline bool isDecoration () const
	{
		return EntityTypes::isDecoration(_type);
	}

	inline bool isParticle () const
	{
		return EntityTypes::isParticle(_type);
	}

	inline bool isWater () const
	{
		return EntityTypes::isWater(_type);
	}

	inline bool isMapTile () const
	{
		return EntityTypes::isMapTile(_type);
	}

	inline bool isGeyser () const
	{
		return EntityTypes::isGeyser(_type);
	}

	inline bool isBorder () const
	{
		return EntityTypes::isBorder(_type);
	}

	inline bool isPlatform () const
	{
		return EntityTypes::isPlatform(_type);
	}

	inline bool isEmitter () const
	{
		return EntityTypes::isEmitter(_type);
	}

	// checks whether the entity is solid. a solid entity will collide with other
	// entities by default
	inline bool isSolid () const
	{
		return EntityTypes::isSolid(_type);
	}

	inline void setVisMask (VisMask vismask)
	{
		_vismask = vismask;
	}

	inline VisMask getVisMask () const
	{
		return _vismask;
	}

	inline const Animation& getAnimationType () const
	{
		return *_animationType;
	}

	inline EntityAlignment getSpriteAlignment () const
	{
		return _spriteAlignment;
	}

	inline int getState () const
	{
		return _state;
	}

	inline void setPosAndAngle (const b2Vec2& pos, float angle)
	{
		for (BodyListConstIterator i = _bodies.begin(); i != _bodies.end(); ++i) {
			(*i)->SetTransform(pos, angle);
		}
	}

	// angle in radians
	inline void setAngle (float angle)
	{
		setPosAndAngle(getPos(), angle);
	}

	// register the physics body
	void addBody (b2Body* body)
	{
		_bodies.push_back(body);

		computeAABB();

		_size = _b2AABB.upperBound - _b2AABB.lowerBound;
		debug(LOG_SERVER, String::format("size: %f:%f, Type: %s", _size.x, _size.y, _type.name.c_str()));
	}

	// returns the physics bodies of the entity
	inline const BodyList& getBodies () const
	{
		return _bodies;
	}

	// Returns the directions the entity is moving in
	Direction getVelocityDirection (IEntity* entity, float vEpsilon = 0.01f) const;

	b2Vec2 getImpactVelocity (b2Contact *contact) const;

	inline bool isImpactVelocityMoreThan (b2Contact* contact, float impactThreshold) const
	{
		const b2Vec2& impactV = getImpactVelocity(contact);
		return fabs(impactV.x) >= impactThreshold || fabs(impactV.y) >= impactThreshold;
	}

	inline void setTouchingWater (bool touchingWater)
	{
		_touchingWater = touchingWater;
	}

	inline void setGravityScale (float gravityScale)
	{
		for (BodyListIterator i = _bodies.begin(); i != _bodies.end(); ++i) {
			(*i)->SetGravityScale(gravityScale);
		}
	}

	inline void toggleGravityScale ()
	{
		float gravityScale = getGravityScale();
		if (fabs(gravityScale) <= 0.000001f) {
			setGravityScale(_oldGravityScale);
		} else {
			_oldGravityScale = gravityScale;
			setGravityScale(0.0f);
		}
	}

	inline void setLinearVelocity (const b2Vec2& v)
	{
		for (BodyListConstIterator i = _bodies.begin(); i != _bodies.end(); ++i) {
			(*i)->SetLinearVelocity(v);
		}
	}

	inline void setAngularVelocity (const float v)
	{
		for (BodyListConstIterator i = _bodies.begin(); i != _bodies.end(); ++i) {
			(*i)->SetAngularVelocity(v);
		}
	}

	inline b2Vec2 getLinearVelocity () const
	{
		if (_bodies.empty())
			return b2Vec2_zero;
		return _bodies[0]->GetLinearVelocity();
	}

	inline float getAngularVelocity () const
	{
		if (_bodies.empty())
			return 0.0f;
		return _bodies[0]->GetAngularVelocity();
	}

	inline float getInertia () const
	{
		if (_bodies.empty())
			return 0.0f;
		return _bodies[0]->GetInertia();
	}

	inline void applyTorque (float torque)
	{
		for (BodyListConstIterator i = _bodies.begin(); i != _bodies.end(); ++i) {
			(*i)->ApplyTorque(torque, true);
		}
	}

	inline b2Vec2 getLinearVelocityFromWorldPoint (const b2Vec2& worldPoint) const
	{
		if (_bodies.empty())
			return b2Vec2_zero;
		return _bodies[0]->GetLinearVelocityFromWorldPoint(worldPoint);
	}

	inline void setLinearDamping (float linearDamping)
	{
		for (BodyListConstIterator i = _bodies.begin(); i != _bodies.end(); ++i) {
			(*i)->SetLinearDamping(linearDamping);
		}
	}

	inline void applyLinearImpulse (const b2Vec2& impulse)
	{
		for (BodyListConstIterator i = _bodies.begin(); i != _bodies.end(); ++i) {
			(*i)->ApplyLinearImpulse(impulse, (*i)->GetWorldCenter(), true);
		}
	}

	inline void setAngularDamping (float angularDamping)
	{
		for (BodyListConstIterator i = _bodies.begin(); i != _bodies.end(); ++i) {
			(*i)->SetAngularDamping(angularDamping);
		}
	}

	inline void applyForceToCenter (const b2Vec2& force)
	{
		for (BodyListConstIterator i = _bodies.begin(); i != _bodies.end(); ++i) {
			(*i)->ApplyForceToCenter(force, true);
		}
	}

	inline bool isTouchingWater () const
	{
		return _touchingWater;
	}

	virtual void clearJoint (b2Joint *joint)
	{
		if (_ropeJoint == joint)
			_ropeJoint = nullptr;
	}

	virtual inline operator std::string () const
	{
		std::stringstream ss;
		ss << "IEntity " << _id << ", vismask: " << _vismask;
		return ss.str();
	}

	virtual void print (std::ostream &stream, int level) const;

	std::string toString () const;
};

inline std::ostream& operator<< (std::ostream &stream, const IEntity &in)
{
	in.print(stream, 0);
	return stream;
}

}
