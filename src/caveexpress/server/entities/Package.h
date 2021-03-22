#pragma once

#include <memory>
#include "common/IMap.h"
#include "caveexpress/server/entities/CollectableEntity.h"

namespace caveexpress {

// forward decl
class Map;
class PackageTarget;

/**
 * @brief The package can get collected by the player or destroyed by
 * aggressive npcs. The player has to deliver it to a package target in the map.
 * Once the package is colliding with a suitable package target, it will
 * automatically get pulled into the target and gets destroyed.
 *
 * @sa PackageTarget
 * @sa Player
 * @sa AggressiveNPC
 * @sa CollectableEntity
 */
class Package: public CollectableEntity {
private:
	gridCoord _x;
	gridCoord _y;
	PackageTarget *_target;
	bool _delivered;
	bool _counted;
	bool _arrived;
	uint32_t _destroyedTime;
	uint32_t _deliveredTime;
	IEntity *_addRopeJointTo;

public:
	Package (Map& map, gridCoord x, gridCoord y);
	virtual ~Package ();

	bool hasTarget (const PackageTarget *target) const;

	// just arrived at the target. returns true if a bonus should be given to the player
	bool setArrived (bool arrived = true);
	bool isArrived () const;

	// the target has processed the arrived package and the package
	// can now be counted as delivered
	void setDelivered (bool delivered = true);
	bool isDelivered () const;

	void setDestroyed (bool destroyed = true);

	void setCounted (bool counted = true);
	bool isCounted () const;

	void createBody ();

	// IEntity
	void update (uint32_t deltaTime) override;
	bool shouldApplyWind () const override;

	// CollectableEntity
	bool isRemove () const override;
	void onContact (b2Contact* contact, IEntity* entity) override;
	void endContact (b2Contact* contact, IEntity* entity) override;
	bool shouldCollide (const IEntity *entity) const override;
};

typedef std::shared_ptr<Package> PackagePtr;

inline bool Package::hasTarget (const PackageTarget *target) const
{
	return _target == target;
}

inline bool Package::isArrived () const
{
	return _arrived;
}

inline void Package::setDelivered (bool delivered)
{
	_delivered = delivered;
	if (_delivered)
		_deliveredTime = _time;
	else
		_deliveredTime = 0;
}

inline bool Package::isDelivered () const
{
	return _delivered;
}

inline void Package::setCounted (bool counted)
{
	if (counted == _counted)
		return;

	_counted = counted;
}

inline bool Package::isCounted () const
{
	return _counted;
}

inline void Package::setDestroyed (bool destroyed)
{
	if (destroyed) {
		setState(EntityState::ENTITY_DESTROYED);
		_destroyedTime = _time;
	} else {
		setState(EntityState::ENTITY_NORMAL);
		_destroyedTime = 0;
	}
}

}
