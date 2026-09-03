#pragma once

#include "common/Config.h"
#include <cstdint>
#include <cmath>
#include <cstddef>

struct PhysicsVec2 {
	float x = 0.0f;
	float y = 0.0f;

	PhysicsVec2 () = default;
	PhysicsVec2 (float x_, float y_) : x(x_), y(y_) {}

	void set (float x_, float y_)
	{
		x = x_;
		y = y_;
	}

	float lengthSquared () const
	{
		return x * x + y * y;
	}

	float length () const
	{
		return std::sqrt(lengthSquared());
	}

	/// Normalize and return the previous length.
	float normalize ()
	{
		const float len = length();
		if (len > 0.0f) {
			x /= len;
			y /= len;
		}
		return len;
	}

	PhysicsVec2 operator- () const
	{
		return PhysicsVec2(-x, -y);
	}

	void operator+= (const PhysicsVec2& v)
	{
		x += v.x;
		y += v.y;
	}

	void operator-= (const PhysicsVec2& v)
	{
		x -= v.x;
		y -= v.y;
	}

	void operator*= (float s)
	{
		x *= s;
		y *= s;
	}
};

inline PhysicsVec2 operator+ (const PhysicsVec2& a, const PhysicsVec2& b)
{
	return PhysicsVec2(a.x + b.x, a.y + b.y);
}

inline PhysicsVec2 operator- (const PhysicsVec2& a, const PhysicsVec2& b)
{
	return PhysicsVec2(a.x - b.x, a.y - b.y);
}

inline PhysicsVec2 operator* (float s, const PhysicsVec2& v)
{
	return PhysicsVec2(s * v.x, s * v.y);
}

inline PhysicsVec2 operator* (const PhysicsVec2& v, float s)
{
	return PhysicsVec2(s * v.x, s * v.y);
}

inline bool operator== (const PhysicsVec2& a, const PhysicsVec2& b)
{
	return a.x == b.x && a.y == b.y;
}

inline bool operator!= (const PhysicsVec2& a, const PhysicsVec2& b)
{
	return !(a == b);
}

inline float physDot (const PhysicsVec2& a, const PhysicsVec2& b)
{
	return a.x * b.x + a.y * b.y;
}

inline float physCross (const PhysicsVec2& a, const PhysicsVec2& b)
{
	return a.x * b.y - a.y * b.x;
}

inline PhysicsVec2 physCross (const PhysicsVec2& a, float s)
{
	return PhysicsVec2(s * a.y, -s * a.x);
}

inline PhysicsVec2 physCross (float s, const PhysicsVec2& a)
{
	return PhysicsVec2(-s * a.y, s * a.x);
}

inline float physDistance (const PhysicsVec2& a, const PhysicsVec2& b)
{
	return (a - b).length();
}

inline float physAbs (float x)
{
	return std::fabs(x);
}

static const PhysicsVec2 PhysicsVec2_zero(0.0f, 0.0f);

struct PhysicsAABB {
	PhysicsVec2 lowerBound;
	PhysicsVec2 upperBound;

	void combine (const PhysicsAABB& aabb)
	{
		lowerBound.x = lowerBound.x < aabb.lowerBound.x ? lowerBound.x : aabb.lowerBound.x;
		lowerBound.y = lowerBound.y < aabb.lowerBound.y ? lowerBound.y : aabb.lowerBound.y;
		upperBound.x = upperBound.x > aabb.upperBound.x ? upperBound.x : aabb.upperBound.x;
		upperBound.y = upperBound.y > aabb.upperBound.y ? upperBound.y : aabb.upperBound.y;
	}

	void combine (const PhysicsAABB& a, const PhysicsAABB& b)
	{
		*this = a;
		combine(b);
	}
};

struct PhysicsColor {
	float r = 0.0f;
	float g = 0.0f;
	float b = 0.0f;
	float a = 1.0f;

	PhysicsColor () = default;
	PhysicsColor (float r_, float g_, float b_, float a_ = 1.0f) : r(r_), g(g_), b(b_), a(a_) {}
};

struct PhysicsTransform {
	PhysicsVec2 p;
	/// Rotation as cosine/sine pair (matches Box2D convention).
	float c = 1.0f;
	float s = 0.0f;

	void setIdentity ()
	{
		p.set(0.0f, 0.0f);
		c = 1.0f;
		s = 0.0f;
	}

	PhysicsVec2 getXAxis () const
	{
		return PhysicsVec2(c, s);
	}

	PhysicsVec2 getYAxis () const
	{
		return PhysicsVec2(-s, c);
	}
};

enum class PhysicsBodyType {
	Static,
	Kinematic,
	Dynamic
};

enum class PhysicsShapeType {
	Circle,
	Edge,
	Polygon,
	Chain,
	Capsule
};

enum class PhysicsPointState {
	Null,
	Add,
	Persist,
	Remove
};

struct PhysicsFilter {
	uint16_t categoryBits = 0x0001;
	uint16_t maskBits = 0xFFFF;
	int16_t groupIndex = 0;
};

struct PhysicsManifoldPoint {
	float normalImpulse = 0.0f;
	float tangentImpulse = 0.0f;
};

struct PhysicsManifold {
	static const int MaxPoints = 2;
	PhysicsManifoldPoint points[MaxPoints];
	int pointCount = 0;
};

struct PhysicsWorldManifold {
	PhysicsVec2 normal;
	PhysicsVec2 points[PhysicsManifold::MaxPoints];
};

struct PhysicsContactImpulse {
	float normalImpulses[PhysicsManifold::MaxPoints]{};
	float tangentImpulses[PhysicsManifold::MaxPoints]{};
	int count = 0;
};

/// Max polygon vertices across backends (Box2D 2.4 uses 8).
static const int PhysicsMaxPolygonVertices = 8;
static const int PhysicsMaxManifoldPoints = PhysicsManifold::MaxPoints;
static const float PhysicsEpsilon = 1.192092896e-07f;

using PhysicsUserData = uintptr_t;
