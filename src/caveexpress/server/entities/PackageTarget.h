#pragma once

#include "caveexpress/server/entities/MapTile.h"
#include <memory>

namespace caveexpress {

// forward decl
class Map;
class Package;

class PackageTarget: public MapTile {
private:
	b2DistanceJoint *_joint;
	Package *_package;
	// millis to reduce the distance joint
	int32_t _lengthUpdate;
	// apply the joint to the given package in order to pull it in
	void applyJoint (Package *package);
	// remove the joint once the package was completly pulled into the packagetarget
	void removeJoint ();
	// update the distance if enough time has passed to slowly pull the package into the target
	void updateJoint (uint32_t deltaTime);

	bool isValidContact (const b2Contact* contact, const std::string& id) const;
	std::string getUserData (const b2Contact* contact) const;
public:
	PackageTarget (Map& map, const std::string& spriteID, gridCoord x, gridCoord y);
	virtual ~PackageTarget ();

	// IEntity
	bool shouldCollide (const IEntity* entity) const override;
	void onPreSolve (b2Contact* contact, IEntity* entity, const b2Manifold* oldManifold) override;
	void update (uint32_t deltaTime) override;
	void clearJoint (b2Joint *joint) override;
};

inline bool PackageTarget::isValidContact (const b2Contact* contact, const std::string& id) const
{
	const std::string userData = getUserData(contact);
	return userData == id;
}

typedef std::shared_ptr<PackageTarget> PackageTargetPtr;

}
