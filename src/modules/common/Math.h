#pragma once

#include <Box2D/Common/b2Math.h>
#include <algorithm>
#include <cmath>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include "common/Compiler.h"
#define GLM_FORCE_RADIANS
GCC_DIAG_OFF(shadow)
GCC_DIAG_OFF(cast-qual)
GCC_DIAG_OFF(cast-align)
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
GCC_DIAG_ON(cast-align)
GCC_DIAG_ON(cast-qual)
GCC_DIAG_ON(shadow)
#include "common/vec2.h"

#define lengthof(x) (sizeof(x) / sizeof(*(x)))
#define lengthofi(x) static_cast<int>(sizeof(x) / sizeof(*(x)))

#ifndef M_PI
#define M_PI 3.14159265358979323846  /* matches value in gcc v2 math.h */
#endif
#ifndef M_PI_2
#define M_PI_2 1.57079632679489661923
#endif

static const double DEG2RAD = M_PI / 180.0;
static const double RAD2DEG = 180.0 / M_PI;

#define FLOAT_CHECK(p) assert(!isinf(p));assert(!isnan(p))

inline bool b2Vec2Equals (const b2Vec2& one, const b2Vec2& two, float epsilon = EPSILON)
{
	return fequals(one.x, two.x, epsilon) && fequals(one.y, two.y, epsilon);
}

// gives random number in the set of {lower, ..., upper}
inline int randBetween (int lower, int upper)
{
	return lower + (int) ((rand() / (RAND_MAX + 1.0)) * (upper - lower + 1));
}

inline float randBetweenf (float lower, float upper)
{
	const float random = ((float) rand()) / (float) RAND_MAX;
	const float diff = upper - lower;
	const float r = random * diff;
	return lower + r;
}

template<typename T>
inline T clamp (T a, T low, T high)
{
	return std::max(low, std::min(a, high));
}

inline double RadiansToDegrees (double radians)
{
	return radians * RAD2DEG;
}

inline double DegreesToRadians (double degrees)
{
	return degrees * DEG2RAD;
}

inline float Round (float r)
{
	return (r > 0.0) ? floor(r + 0.5) : ceil(r - 0.5);
}

template<class T>
inline bool Between (T value, T min, T max)
{
	return value >= min && value <= max;
}

typedef float Color[4];

static const Color colorGrayAlpha40 = { 0.976f, 0.976f, 0.976f, 0.4f };
static const Color colorGrayAlpha = { 0.976f, 0.976f, 0.976f, 0.8f };
static const Color colorGray = { 0.6f, 0.6f, 0.6f, 1.0f };
static const Color colorWhiteAlpha80 = { 1.0f, 1.0f, 1.0f, 0.8f };
static const Color colorWhite = { 1.0f, 1.0f, 1.0f, 1.0f };
static const Color colorGreen = { 0.0f, 1.0f, 0.0f, 1.0f };
static const Color colorBrightGreen = { 0.0f, 0.6f, 0.0f, 1.0f };
static const Color colorBlue = { 0.0f, 0.0f, 1.0f, 1.0f };
static const Color colorBrightBlue = { 0.0f, 0.0f, 0.6f, 1.0f };
static const Color colorRed = { 1.0f, 0.0f, 0.0f, 1.0f };
static const Color colorBrightRed = { 0.6f, 0.0f, 0.0f, 1.0f };
static const Color colorBlack = { 0.0f, 0.0f, 0.0f, 1.0f };
static const Color colorDark = { 0.2f, 0.2f, 0.2f, 1.0f };
static const Color colorNull = { 0.0f, 0.0f, 0.0f, 0.0f };
static const Color backgroundColor = { 1.0f, 1.0f, 1.0f, 0.2f };
static const Color highlightColor = { 1.0f, 1.0f, 1.0f, 0.5f };
static const Color colorYellow = { 1.0f, 1.0f, 0.0f, 1.0f };
static const Color colorCyan = { 0.2313f, 0.7372f, 0.8274f, 1.0f };

inline void FadeIn (float& value, float frac)
{
	value *= frac;
}

inline void FadeOut (float& value, float frac)
{
	value *= (1.0 - frac);
}

inline void FadeSin (float& value, float frac)
{
	value *= sin(frac * M_PI);
}

inline void FadeSaw (float& value, float frac)
{
	if (frac < 0.5)
		frac *= 2.0f;
	else
		frac = (1.0 - frac) * 2.0f;
	FadeIn(value, frac);
}

inline void Vector4Set (const Color& in, Color& out)
{
	out[0] = in[0];
	out[1] = in[1];
	out[2] = in[2];
	out[3] = in[3];
}

// returns the angle between the given two points and the horizontal axis in degree
inline double getAngleBetweenPoints (double x1, double y1, double x2, double y2)
{
	const double dX = x2 - x1;
	const double dY = y2 - y1;
	const double angleInDegrees = ::atan2(dY, dX) * RAD2DEG;
	return angleInDegrees;
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
