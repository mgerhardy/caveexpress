#pragma once

#include "common/EntityType.h"

namespace EntityTypes {
extern EntityType SOLID;
extern EntityType GROUND;
extern EntityType PLAYER;
extern EntityType PACKAGE;
extern EntityType TARGET;

inline bool isGround (const EntityType& other)
{
	return other == GROUND;
}

inline bool isPlayer (const EntityType& other)
{
	return other == PLAYER;
}

inline bool isTarget (const EntityType& other)
{
	return other == TARGET;
}

inline bool isPackage (const EntityType& other)
{
	return other == PACKAGE;
}

inline bool isSolid (const EntityType& other)
{
	return other == SOLID || isGround(other) || isTarget(other);
}

inline bool isMapTile (const EntityType& other)
{
	return isSolid(other);
}

inline bool isDynamic (const EntityType& other)
{
	return isPlayer(other) || isPackage(other);
}

}
