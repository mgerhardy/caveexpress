#pragma once

#include "common/EntityType.h"

namespace miniracer {

namespace EntityTypes {
extern EntityType ROAD;
extern EntityType MOVEABLE;
extern EntityType LAND;
extern EntityType SOLID;
extern EntityType DECAL;
extern EntityType PLAYER;

inline bool isPlayer (const EntityType& other)
{
	return other == PLAYER;
}

inline bool isSolid (const EntityType& other)
{
	return other == SOLID;
}

inline bool isRoad (const EntityType& other)
{
	return other == ROAD;
}

inline bool isMoveable (const EntityType& other)
{
	return other == MOVEABLE;
}

inline bool isLand (const EntityType& other)
{
	return other == LAND;
}

inline bool isMapTile (const EntityType& other)
{
	return isSolid(other) || isRoad(other) || isLand(other) || isMoveable(other);
}

inline bool isDynamic (const EntityType& other)
{
	return isPlayer(other);
}

}

}
