#pragma once

#include "common/SpriteType.h"

namespace miniracer {

namespace SpriteTypes {
extern SpriteType SOLID;
extern SpriteType MOVEABLE;
extern SpriteType ROAD;
extern SpriteType DECAL;
extern SpriteType VEHICLE;
extern SpriteType LAND;
extern SpriteType LIGHTS;
extern SpriteType RACER;

inline bool isSolid (const SpriteType& other)
{
	return other == SOLID;
}

inline bool isLights (const SpriteType& other)
{
	return other == LIGHTS;
}

inline bool isRoad (const SpriteType& other)
{
	return other == ROAD;
}

// put onto the ground - only graphical - no physics impact
inline bool isDecal (const SpriteType& other)
{
	return other == DECAL;
}

// solid but the vehicle can move this object
inline bool isMoveable (const SpriteType& other)
{
	return other == MOVEABLE;
}

inline bool isVehicle (const SpriteType& other)
{
	return other == VEHICLE;
}

// vehicle is slower here
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
