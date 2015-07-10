#pragma once

#include "common/SpriteType.h"

namespace miniracer {

namespace SpriteTypes {
extern SpriteType SOLID;

inline bool isSolid (const SpriteType& other)
{
	return other == SOLID;
}

inline bool isMapTile (const SpriteType& other)
{
	return isSolid(other);
}

}

}
