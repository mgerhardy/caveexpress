#pragma once

#include "caveexpress/server/entities/CollectableEntity.h"

namespace caveexpress {

class Map;

/**
 * @brief Collecting a fruit will give you some energy back
 */
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

}
