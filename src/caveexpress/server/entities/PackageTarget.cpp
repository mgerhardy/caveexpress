#include "PackageTarget.h"
#include "caveexpress/server/map/Map.h"
#include "caveexpress/server/entities/Package.h"
#include "caveexpress/shared/CaveExpressSoundType.h"
#include "caveexpress/shared/constants/Density.h"
#include <SDL_assert.h>

namespace caveexpress {

#define LENGTH_UPDATE_DELAY 50

PackageTarget::PackageTarget (Map& map, const std::string& spriteID, gridCoord x, gridCoord y) :
		MapTile(map, spriteID, x, y,
				ThemeTypes::isIce(map.getTheme()) ? EntityTypes::PACKAGETARGET_ICE : EntityTypes::PACKAGETARGET_ROCK),
		_joint(), _package(nullptr), _lengthUpdate(0)
{
	setAnimationType(Animations::ANIMATION_IDLE);
}

PackageTarget::~PackageTarget ()
{
}

bool PackageTarget::shouldCollide (const IEntity* entity) const
{
	if (!entity->isPackage())
		return false;

	const Package *package = assert_cast<const Package*, const IEntity*>(entity);
	return package->hasTarget(this);
}

void PackageTarget::onPreSolve (PhysicsContact contact, IEntity* entity, const PhysicsManifold& oldManifold)
{
	IEntity::onPreSolve(contact, entity, oldManifold);
	// there is already a pull in progress
	if (_joint.isValid())
		return;

	if (!isValidContact(contact, "top"))
		return;

	contact.setEnabled(false);

	if (entity->isPackage()) {
		Package *package = assert_cast<Package*, IEntity*>(entity);
		package->setLinearVelocity(PhysicsVec2_zero);
		package->setAngularVelocity(0.0f);
		setAnimationType(Animations::ANIMATION_ACTIVE);
		_package = package;
		
		PhysicsVec2 p = getPos();
		p.y -= 2.f;  // louder
		_map.sendSound(getVisMask(), SoundTypes::SOUND_PACKAGE_TARGET, p);
	}
}

std::string PackageTarget::getUserData (PhysicsContact contact) const
{
	PhysicsFixture fixture;
	if (contact.getFixtureA().getBody().getUserData() == (uintptr_t)this) {
		fixture = contact.getFixtureA();
	} else {
		fixture = contact.getFixtureB();
	}
	return reinterpret_cast<const char*>(fixture.getUserData());
}

void PackageTarget::update (uint32_t deltaTime)
{
	IEntity::update(deltaTime);
	if (_package == nullptr)
		return;

	if (!_joint.isValid()) {
		applyJoint(_package);
	} else {
		updateJoint(deltaTime);
	}
}

void PackageTarget::clearJoint (PhysicsJoint joint)
{
	IEntity::clearJoint(joint);
	if (_joint == joint) {
		removeJoint();
	}
}

void PackageTarget::updateJoint (uint32_t deltaTime)
{
	SDL_assert(_joint.isValid());
	const float currentLength = _joint.getLength();
	if (currentLength < 0.05f) {
		removeJoint();
		return;
	}
	_lengthUpdate -= deltaTime;
	if (_lengthUpdate > 0)
		return;

	_lengthUpdate = LENGTH_UPDATE_DELAY;
	_joint.setLength(currentLength - 0.1f);
}

void PackageTarget::removeJoint ()
{
	// the joint is deleted once the body is removed
	_joint.clear();
	setAnimationType(Animations::ANIMATION_IDLE);
	_package->setDelivered();
	_package = nullptr;
}

void PackageTarget::applyJoint (Package *package)
{
	SDL_assert(!_joint.isValid());
	const PhysicsVec2 shift(0.0f, -0.1f);
	PhysicsBody bodyA = getBodies()[0];
	PhysicsBody bodyB = package->getBodies()[0];

	PhysicsDistanceJointDef def;
	def.useWorldAnchors = true;
	def.bodyA = bodyA;
	def.bodyB = bodyB;
	def.worldAnchorA = bodyA.getWorldPoint(shift);
	def.worldAnchorB = package->getPos();
	def.minLength = 0.0f;
	def.maxLength = physDistance(def.worldAnchorA, def.worldAnchorB);
	_joint = _map.getWorld()->createDistanceJoint(def);

	_lengthUpdate = LENGTH_UPDATE_DELAY;
	package->setGravityScale(0.0f);
	const bool bonus = package->setArrived();
	if (!bonus)
		return;
	_map.addPoints(package->getLastDropper(), 30);
}

}
