#pragma once

#include <SDL_assert.h>
#ifndef b2Assert
#define b2Assert SDL_assert
#endif
#include "common/Compiler.h"
GCC_DIAG_OFF(shadow)
#include <Common/b2Math.h>
GCC_DIAG_ON(shadow)
#include "common/Math.h"

inline bool b2Vec2Equals (const b2Vec2& one, const b2Vec2& two, float epsilon = EPSILON)
{
	return fequals(one.x, two.x, epsilon) && fequals(one.y, two.y, epsilon);
}

// Assuming the polygon is simple, checks if it is convex
inline bool isConvex (const b2Vec2 *points, const size_t amount)
{
	if (amount < 4)
		return true;
	bool isPositive = false;
	for (size_t k = 0u; k < amount; ++k) {
		const float dx1 = points[k + 1].x - points[k].x;
		const float dy1 = points[k + 1].y - points[k].y;
		const float dx2 = points[k + 2].x - points[k + 1].x;
		const float dy2 = points[k + 2].y - points[k + 1].y;
		const float cross = dx1 * dy2 - dy1 * dx2;
		// Cross product should have same sign
		// for each vertex if polygon is convex.
		const bool newIsP = cross > 0;
		if (k == 0) {
			isPositive = newIsP;
		} else if (isPositive != newIsP) {
			return false;
		}
	}
	return true;
}
