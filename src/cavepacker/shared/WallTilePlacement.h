#pragma once

#include <string>

namespace cavepacker {

/**
 * Wall sprite placement constraint (cavepacker only).
 * Each rock tile art asset has exactly one orientation; the map loader picks a
 * sprite whose placement matches the wall cell's neighborhood.
 *
 * - left/right/top/down: exactly one playable (non-wall) neighbor in that direction
 * - full: all four neighbors are walls (interior of a thick wall)
 * - any: fallback / ambiguous (corners, exterior-only edges, unconstrained art)
 */
enum class WallPlacement {
	Any,
	Left,
	Right,
	Top,
	Down,
	Full
};

inline const char* wallPlacementToString (WallPlacement placement)
{
	switch (placement) {
	case WallPlacement::Left:
		return "left";
	case WallPlacement::Right:
		return "right";
	case WallPlacement::Top:
		return "top";
	case WallPlacement::Down:
		return "down";
	case WallPlacement::Full:
		return "full";
	case WallPlacement::Any:
	default:
		return "any";
	}
}

inline WallPlacement wallPlacementFromString (const std::string& value)
{
	if (value == "left")
		return WallPlacement::Left;
	if (value == "right")
		return WallPlacement::Right;
	if (value == "top")
		return WallPlacement::Top;
	if (value == "down")
		return WallPlacement::Down;
	if (value == "full")
		return WallPlacement::Full;
	return WallPlacement::Any;
}

/**
 * @param openL/R/T/D true if that neighbor is playable (floor/target/package/...)
 * @param wallL/R/T/D true if that neighbor is a wall tile
 */
inline WallPlacement computeWallPlacement (bool openL, bool openR, bool openT, bool openD, bool wallL, bool wallR,
		bool wallT, bool wallD)
{
	const int openCount = (openL ? 1 : 0) + (openR ? 1 : 0) + (openT ? 1 : 0) + (openD ? 1 : 0);
	if (openCount == 1) {
		if (openL)
			return WallPlacement::Left;
		if (openR)
			return WallPlacement::Right;
		if (openT)
			return WallPlacement::Top;
		return WallPlacement::Down;
	}
	if (openCount == 0 && wallL && wallR && wallT && wallD)
		return WallPlacement::Full;
	return WallPlacement::Any;
}

}
