#pragma once

#include "common/SpriteDefinition.h"

namespace caveexpress {

/**
 * Collision-shape derived roles for mapgen bucketing.
 * Derived from SpriteDef polygons (Lua), not tile id strings.
 */
struct SpriteShapeTraits {
	bool fullSolid = false;   // fills (nearly) the whole 1x1 tile
	bool thinTopSlab = false; // hanging deck: thin solid along the top
	bool shim = false;        // small downward peak under rock
	bool undercutL = false;   // top-biased bite, open lower-left
	bool undercutR = false;   // top-biased bite, open lower-right
	bool slopeL = false;      // walkable ramp, bottom-filled, open upper-left
	bool slopeR = false;      // walkable ramp, bottom-filled, open upper-right
};

SpriteShapeTraits analyzeSpriteShape (const SpriteDefPtr& def);

}
