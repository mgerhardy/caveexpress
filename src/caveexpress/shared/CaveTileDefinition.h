#pragma once

#include "common/MapTileDefinition.h"
#include "common/EntityType.h"

namespace caveexpress {

class CaveTileDefinition: public MapTileDefinition {
public:
	// the type of the npc that this tile will spawn
	const EntityType* type;
	// the delay of the npc spawn for this tile
	int delay;

	CaveTileDefinition (gridCoord _x, gridCoord _y, const SpriteDefPtr &_spriteDef, const EntityType& _type, int _delay) :
			MapTileDefinition(_x, _y, _spriteDef, 0), type(&_type), delay(_delay)
	{
	}
};

}
