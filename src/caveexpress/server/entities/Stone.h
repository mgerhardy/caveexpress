#pragma once

#include "caveexpress/server/entities/CollectableEntity.h"
#include "common/IMap.h"
#include <memory>

namespace caveexpress {

// forward decl
class Map;

/**
 * @brief The stone - throw it at trees or daze aggressive npcs by throwing it at them.
 * Can also be useful to be placed next to a package target for easier throwing of packages.
 */
class Stone: public CollectableEntity {
private:
	gridCoord _x;
	gridCoord _y;

public:
	Stone (Map& map, gridCoord x, gridCoord y, const IEntity *creator = nullptr);
	virtual ~Stone ();

	void createBody ();

	// IEntity
	bool shouldCollide (const IEntity *entity) const override;
	void onPreSolve (b2Contact* contact, IEntity* entity, const b2Manifold* oldManifold) override;
	void onContact (b2Contact* contact, IEntity* entity) override;
	void endContact (b2Contact* contact, IEntity* entity) override;
};

typedef std::shared_ptr<Stone> StonePtr;

}
