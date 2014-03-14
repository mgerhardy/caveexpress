#pragma once

#include "engine/common/SpriteType.h"

namespace SpriteTypes {
extern SpriteType WATERFALL;
extern SpriteType ROCK;
extern SpriteType CAVE;
extern SpriteType BRIDGE_LEFT;
extern SpriteType BRIDGE_RIGHT;
extern SpriteType BRIDGE_PLANK;
extern SpriteType GROUND;
extern SpriteType GROUND_LEFT;
extern SpriteType GROUND_RIGHT;
extern SpriteType SLOPE_LEFT;
extern SpriteType SLOPE_RIGHT;
extern SpriteType WINDOW;
extern SpriteType LAVA;
extern SpriteType BACKGROUND;
extern SpriteType CAVE_SIGN;
extern SpriteType LIANE;
extern SpriteType GEYSER_ICE;
extern SpriteType GEYSER_ROCK;
extern SpriteType PACKAGETARGET_ICE;
extern SpriteType PACKAGETARGET_ROCK;

inline bool isBackground (const SpriteType& other)
{
	return other == BACKGROUND;
}

inline bool isWaterFall (const SpriteType& other)
{
	return other == WATERFALL;
}

inline bool isCave (const SpriteType& other)
{
	return other == CAVE;
}

inline bool isRock (const SpriteType& other)
{
	return other == ROCK;
}

inline bool isLiane (const SpriteType& other)
{
	return other == LIANE;
}

inline bool isLava (const SpriteType& other)
{
	return other == LAVA;
}

inline bool isWindow (const SpriteType& other)
{
	return other == WINDOW;
}

inline bool isGround (const SpriteType& other)
{
	return other == GROUND;
}

inline bool isGeyser (const SpriteType& other)
{
	return other == GEYSER_ICE || other == GEYSER_ROCK;
}

inline bool isPackageTargetIce (const SpriteType& other)
{
	return other == PACKAGETARGET_ICE;
}

inline bool isPackageTargetRock (const SpriteType& other)
{
	return other == PACKAGETARGET_ROCK;
}

inline bool isGroundLeft (const SpriteType& other)
{
	return other == GROUND_LEFT;
}

inline bool isGroundRight (const SpriteType& other)
{
	return other == GROUND_RIGHT;
}

inline bool isBridgeLeft (const SpriteType& other)
{
	return other == BRIDGE_LEFT;
}

inline bool isBridgeRight (const SpriteType& other)
{
	return other == BRIDGE_RIGHT;
}

inline bool isBridgePlank (const SpriteType& other)
{
	return other == BRIDGE_PLANK;
}

inline bool isSlopeRight (const SpriteType& other)
{
	return other == SLOPE_RIGHT;
}

inline bool isSlopeLeft (const SpriteType& other)
{
	return other == SLOPE_LEFT;
}

inline bool isSlope (const SpriteType& other)
{
	return isSlopeRight(other) || isSlopeLeft(other);
}

inline bool isPackageTarget (const SpriteType& other)
{
	return isPackageTargetIce(other) || isPackageTargetRock(other);
}

inline bool isBridge (const SpriteType& other)
{
	return isBridgeLeft(other) || isBridgeRight(other) || isBridgePlank(other);
}

inline bool isAnyGround (const SpriteType& other)
{
	return isGround(other) || isGroundLeft(other) || isGroundRight(other);
}

inline bool isSolid (const SpriteType& other)
{
	return isRock(other) || isAnyGround(other) || isWaterFall(other) || isBridge(other) || isGeyser(other)
			|| isPackageTarget(other) || isSlope(other) || isLava(other);
}

inline bool isMapTile (const SpriteType& other)
{
	return isSolid(other) || isCave(other) || isBackground(other) || isWindow(other);
}

}
