#pragma once

#include "common/SpriteType.h"

namespace miniracer {

namespace SpriteTypes {
extern SpriteType SOLID;
extern SpriteType ROAD;
extern SpriteType DECAL;
extern SpriteType VEHICLE;
extern SpriteType LAND;

inline bool isSolid (const SpriteType& other)
{
	return other == SOLID;
}

inline bool isRoad (const SpriteType& other)
{
	return other == ROAD;
}

inline bool isDecal (const SpriteType& other)
{
	return other == DECAL;
}

inline bool isVehicle (const SpriteType& other)
{
	return other == VEHICLE;
}

inline bool isLand (const SpriteType& other)
{
	return other == LAND;
}

inline bool isMapTile (const SpriteType& other)
{
	return isSolid(other) || isLand(other) || isRoad(other) || isDecal(other);
}

}

}
