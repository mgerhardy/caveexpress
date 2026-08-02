#include "Package.h"
#include "caveexpress/server/entities/Tree.h"
#include "caveexpress/server/entities/npcs/NPCAttacking.h"
#include "caveexpress/server/entities/npcs/NPCFlying.h"
#include "caveexpress/server/map/Map.h"
#include "caveexpress/shared/constants/Density.h"
#include "caveexpress/shared/CaveExpressSoundType.h"

namespace caveexpress {

#define DELIVERED_REMOVE_TIME 10000
#define DESTROYED_REMOVE_TIME 5000

Package::Package (Map& map, gridCoord x, gridCoord y) :
		CollectableEntity(ThemeTypes::isIce(map.getTheme()) ? EntityTypes::PACKAGE_ICE : EntityTypes::PACKAGE_ROCK, map), _x(x), _y(
				y), _target(nullptr), _delivered(false), _counted(false), _arrived(false), _destroyedTime(0), _deliveredTime(
				0), _addRopeJointTo(nullptr)
{
}

Package::~Package ()
{
}

bool Package::shouldApplyWind () const
{
	return true;
}

void Package::createBody ()
{
	PhysicsFixtureDef fd;
	fd.shapeType = PhysicsShapeType::Polygon;
	fd.useBox = true;
	fd.boxHalfWidth = _size.x / 2.0f;
	fd.boxHalfHeight = _size.y / 2.0f;
	fd.density = DENSITY_PACKAGE;
	fd.friction = 1.0f;
	fd.restitution = 0.01f;

	PhysicsBodyDef bd;
	bd.position.set(_x, _y);
	bd.type = PhysicsBodyType::Dynamic;

	_map.addToWorld(fd, bd, this);
	_map.addEntity(this);
}

void Package::update (uint32_t deltaTime)
{
	CollectableEntity::update(deltaTime);
	if (_addRopeJointTo) {
		addRopeJoint(_addRopeJointTo);
		_addRopeJointTo = nullptr;
	}

	if ((_arrived || _delivered || isDestroyed()) && _ropeJoint.isValid()) {
		removeRopeJoint();
	}

	if (isCollected()) {
		setLinearDamping(0.5f);
		setAngularDamping(0.5f);
	}

	if (!_target) {
		_target = _map.getPackageTarget();
		// if we can't find a package target, we will destroy the package
		if (!_target)
			setDestroyed();
	}

	if (!isCounted() && isDelivered()) {
		_map.countTransferedPackage();
		setCounted();
	}
}

bool Package::isRemove () const
{
	// skip CollectableEntity - because we don't want to remove the package immediately even if it's collected
	if (IEntity::isRemove())
		return true;
	if (isDestroyed() && _time > _destroyedTime + DESTROYED_REMOVE_TIME)
		return true;
	return isCounted();
}

void Package::onContact (PhysicsContact contact, IEntity* entity)
{
	const bool oldCollectedState = isCollected();
	CollectableEntity::onContact(contact, entity);

	if (isImpactVelocityMoreThan(contact, 1.1f))
		_map.sendSound(getVisMask(), SoundTypes::SOUND_PACKAGE_COLLIDE, getPos());

	if (!oldCollectedState && isCollected()) {
		_addRopeJointTo = entity;
	} else {
		if (entity->isSolid() || entity->isStone() || entity->isPackage()) {
			setLinearDamping(3.0f);
			setAngularDamping(1.0f);
		} else if (entity->isBorder()) {
			const Border *b = assert_cast<const Border*, const IEntity*>(entity);
			if (!b->isTop()) {
				setLinearDamping(4.0f);
				setAngularDamping(1.0f);
			}
		} else if (entity->isNpcAttacking()) {
			setDestroyed(true);
		}
	}
}

void Package::endContact (PhysicsContact contact, IEntity* entity)
{
	CollectableEntity::endContact(contact, entity);

	if (entity->isSolid() || entity->isPackage() || entity->isStone() || entity->isBorder()) {
		setLinearDamping(0.0f);
		setAngularDamping(0.0);
	}
}

bool Package::setArrived (bool arrived)
{
	if (arrived == _arrived)
		return false;

	_arrived = arrived;

	if (_collectCount == 1 && _arrived) {
		const uint32_t deltaSeconds = _time - _collectedTime;
		return deltaSeconds < 1000;
	}

	return false;
}

bool Package::shouldCollide (const IEntity *entity) const
{
	if (isDestroyed())
		return false;

	if (entity->isPlayer()) {
		const Player* player = assert_cast<const Player*, const IEntity*>(entity);
		return !player->isCrashed() && !isArrived() && !isDelivered() && entity->getPos().y < getPos().y;
	}

	if (entity->isPackageTarget() || entity->isWater()) {
		return !isArrived() && !isDelivered();
	}

	if (entity->isNpcAttacking()) {
		const NPCAttacking *npc = assert_cast<const NPCAttacking*, const IEntity*>(entity);
		return npc->isAttacking();
	}

	if (entity->isStone())
		return true;

	if (entity->isPackage()) {
		const Package* package = assert_cast<const Package*, const IEntity*>(entity);
		return !package->isDestroyed();
	}

	return CollectableEntity::shouldCollide(entity);
}

}
