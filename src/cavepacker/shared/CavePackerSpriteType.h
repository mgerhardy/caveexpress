#pragma once

#include "engine/common/SpriteType.h"

namespace SpriteTypes {
extern SpriteType ROCK;
extern SpriteType GROUND;
extern SpriteType BACKGROUND;

inline bool isBackground (const SpriteType& other)
{
	return other == BACKGROUND;
}

inline bool isRock (const SpriteType& other)
{
	return other == ROCK;
}

inline bool isGround (const SpriteType& other)
{
	return other == GROUND;
}

inline bool isSolid (const SpriteType& other)
{
	return isRock(other) || isGround(other);
}

inline bool isMapTile (const SpriteType& other)
{
	return isSolid(other) || isBackground(other);
}

}
