#pragma once

#include "engine/common/SpriteType.h"

namespace SpriteTypes {
extern SpriteType SOLID;
extern SpriteType GROUND;

inline bool isSolid (const SpriteType& other)
{
	return other == SOLID;
}

inline bool isGround (const SpriteType& other)
{
	return other == GROUND;
}

inline bool isMapTile (const SpriteType& other)
{
	return isSolid(other) || isGround(other);
}

}
