#include "CollectableEntity.h"

namespace caveexpress {

// forward decl
class Map;

/**
 * @brief Collecting the egg makes you invulnerable for some time.
 */
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

}
