#pragma once

#include "common/EntityType.h"

namespace miniracer {

namespace EntityTypes {
extern EntityType SOLID;
extern EntityType PLAYER;

inline bool isPlayer (const EntityType& other)
{
	return other == PLAYER;
}

inline bool isSolid (const EntityType& other)
{
	return other == SOLID;
}

inline bool isMapTile (const EntityType& other)
{
	return isSolid(other);
}

inline bool isDynamic (const EntityType& other)
{
	return isPlayer(other);
}

}

}
