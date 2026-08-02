#pragma once

#include "physics/PhysicsTypes.h"
#include "common/Math.h"

inline bool physVec2Equals (const PhysicsVec2& one, const PhysicsVec2& two, float epsilon = EPSILON)
{
	return fequals(one.x, two.x, epsilon) && fequals(one.y, two.y, epsilon);
}

/// Assuming the polygon is simple, checks if it is convex.
inline bool physIsConvex (const PhysicsVec2 *points, const size_t amount)
{
	if (amount < 4)
		return true;
	bool isPositive = false;
	for (size_t k = 0u; k < amount; ++k) {
		const float dx1 = points[(k + 1) % amount].x - points[k].x;
		const float dy1 = points[(k + 1) % amount].y - points[k].y;
		const float dx2 = points[(k + 2) % amount].x - points[(k + 1) % amount].x;
		const float dy2 = points[(k + 2) % amount].y - points[(k + 1) % amount].y;
		const float cross = dx1 * dy2 - dy1 * dx2;
		const bool newIsP = cross > 0;
		if (k == 0) {
			isPositive = newIsP;
		} else if (isPositive != newIsP) {
			return false;
		}
	}
	return true;
}

// Backwards-compatible aliases used during migration.
inline bool b2Vec2Equals (const PhysicsVec2& one, const PhysicsVec2& two, float epsilon = EPSILON)
{
	return physVec2Equals(one, two, epsilon);
}

inline bool isConvex (const PhysicsVec2 *points, const size_t amount)
{
	return physIsConvex(points, amount);
}
