#pragma once

#include "caveexpress/server/entities/CollectableEntity.h"
#include "engine/common/IMap.h"
#include "engine/common/Pointers.h"

// forward decl
class Map;

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

typedef SharedPtr<Stone> StonePtr;
