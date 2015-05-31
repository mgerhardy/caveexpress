#pragma once

#include "common/IMap.h"
#include "common/SpriteDefinition.h"

class MapTileDefinition {
public:
	// the coordinates of the emitter
	gridCoord x;
	gridCoord y;
	// the sprite of this tile
	SpriteDefPtr spriteDef;
	// the angle this tile will get added to the world with
	EntityAngle angle;

	MapTileDefinition (gridCoord _x, gridCoord _y, const SpriteDefPtr &_spriteDef, EntityAngle _angle) :
			x(_x), y(_y), spriteDef(_spriteDef), angle(_angle)
	{
	}

	// comparator for sorting the tiles in the map for faster access
	inline bool operator < (const MapTileDefinition& o) const
	{
		return x < o.x && y < o.y;
	}

	// checks whether the given grid coordinates intersect with this tile
	inline bool intersects (gridCoord _x, gridCoord _y) const
	{
		if (_x < x)
			return false;

		if (_y < y)
			return false;

		if (_x >= x + spriteDef->width)
			return false;

		if (_y >= y + spriteDef->height)
			return false;

		return true;
	}
};
