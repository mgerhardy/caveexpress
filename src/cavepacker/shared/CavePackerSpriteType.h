#pragma once

#include "common/SpriteType.h"

namespace cavepacker {

namespace SpriteTypes {
extern SpriteType SOLID;
extern SpriteType GROUND;
extern SpriteType TARGET;
extern SpriteType PACKAGE;

inline bool isSolid (const SpriteType& other)
{
	return other == SOLID;
}

inline bool isTarget (const SpriteType& other)
{
	return other == TARGET;
}

inline bool isPackage (const SpriteType& other)
{
	return other == PACKAGE;
}

inline bool isGround (const SpriteType& other)
{
	return other == GROUND;
}

inline bool isMapTile (const SpriteType& other)
{
	return isSolid(other) || isGround(other) || isTarget(other);
}

}

}
