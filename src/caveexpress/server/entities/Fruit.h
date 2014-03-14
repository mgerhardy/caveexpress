#pragma once

#include "caveexpress/server/entities/CollectableEntity.h"

class Map;

class Fruit: public CollectableEntity {
protected:
	gridCoord _x;
	gridCoord _y;

public:
	Fruit(Map& map, const EntityType& type, gridCoord x, gridCoord y);
	virtual ~Fruit();

	void createBody ();

	void onSpawn () override;
};
