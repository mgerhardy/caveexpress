#include "CollectableEntity.h"

class Map;

class Egg: public CollectableEntity {
protected:
	gridCoord _x;
	gridCoord _y;

public:
	Egg (Map& map, gridCoord x, gridCoord y);
	virtual ~Egg ();

	void createBody ();

	void onSpawn () override;
};
