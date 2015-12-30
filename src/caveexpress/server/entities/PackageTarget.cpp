#include "PackageTarget.h"
#include "caveexpress/server/map/Map.h"
#include "caveexpress/server/entities/Package.h"
#include "caveexpress/shared/constants/Density.h"
#include <SDL_assert.h>

namespace caveexpress {

#define LENGTH_UPDATE_DELAY 500

PackageTarget::PackageTarget (Map& map, const std::string& spriteID, gridCoord x, gridCoord y) :
		MapTile(map, spriteID, x, y,
				ThemeTypes::isIce(map.getTheme()) ? EntityTypes::PACKAGETARGET_ICE : EntityTypes::PACKAGETARGET_ROCK), _joint(
				nullptr), _package(nullptr), _lengthUpdate(0)
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

	const Package *package = static_cast<const Package*>(entity);
	return package->hasTarget(this);
}

void PackageTarget::onPreSolve (b2Contact* contact, IEntity* entity, const b2Manifold* oldManifold)
{
	IEntity::onPreSolve(contact, entity, oldManifold);
	// there is already a pull in progress
	if (_joint != nullptr)
		return;

	if (!isValidContact(contact, "top"))
		return;

	contact->SetEnabled(false);

	if (entity->isPackage()) {
		Package *package = static_cast<Package*>(entity);
		package->setLinearVelocity(b2Vec2_zero);
		package->setAngularVelocity(0.0f);
		setAnimationType(Animations::ANIMATION_ACTIVE);
		_package = package;
	}
}

std::string PackageTarget::getUserData (const b2Contact* contact) const
{
	const b2Fixture* fixture;
	if (contact->GetFixtureA()->GetBody()->GetUserData() == this) {
		fixture = contact->GetFixtureA();
	} else {
		fixture = contact->GetFixtureB();
	}
	return static_cast<const char*>(fixture->GetUserData());
}

void PackageTarget::update (uint32_t deltaTime)
{
	IEntity::update(deltaTime);
	if (_package == nullptr)
		return;

	if (_joint == nullptr) {
		applyJoint(_package);
	} else {
		updateJoint(deltaTime);
	}
}

void PackageTarget::clearJoint (b2Joint *joint)
{
	IEntity::clearJoint(joint);
	if (_joint == joint) {
		removeJoint();
	}
}

void PackageTarget::updateJoint (uint32_t deltaTime)
{
	SDL_assert(_joint != nullptr);
	const float currentLength = _joint->GetLength();
	if (currentLength < 0.05f) {
		removeJoint();
		return;
	}
	_lengthUpdate -= deltaTime;
	if (_lengthUpdate > 0)
		return;

	_lengthUpdate = LENGTH_UPDATE_DELAY;
	_joint->SetLength(currentLength - 0.1f);
}

void PackageTarget::removeJoint ()
{
	// the joint is deleted once the body is removed
	_joint = nullptr;
	setAnimationType(Animations::ANIMATION_IDLE);
	_package->setDelivered();
	_package = nullptr;
}

void PackageTarget::applyJoint (Package *package)
{
	SDL_assert(_joint == nullptr);
	const b2Vec2 shift(0.0f, -0.1f);
	b2DistanceJointDef def;
	def.Initialize(getBodies()[0], package->getBodies()[0], getBodies()[0]->GetWorldPoint(shift), package->getBodies()[0]->GetPosition());
	_joint = static_cast<b2DistanceJoint*>(_map.getWorld()->CreateJoint(&def));
	_lengthUpdate = LENGTH_UPDATE_DELAY;
	package->setGravityScale(0.0f);
	const bool bonus = package->setArrived();
	if (!bonus)
		return;
	_map.addPoints(package->getLastDropper(), 30);
}

}
