#pragma once

#define EPSILON 0.000001f
inline bool fequals(float v1, float v2, float epsilon = EPSILON) {
	return fabs(v1 - v2) < epsilon;
}

class vec2 {
public:
	vec2() :
			x(0.0f), y(0.0f) {
	}
	vec2(float _x, float _y) :
			x(_x), y(_y) {
	}
	vec2 & operator=(const vec2 &rhs) {
		x = rhs.x;
		y = rhs.y;
		return *this;
	}
	bool operator==(const vec2 &other) const {
		return fequals(x, other.x) && fequals(y, other.y);
	}

	bool isZero(float epsilon = EPSILON) const {
		return fequals(x, 0.0f, epsilon) && fequals(y, 0.0f, epsilon);
	}

	vec2 operator -() const {
		const vec2 v(-x, -y);
		return v;
	}

	void operator +=(const vec2& v) {
		x += v.x;
		y += v.y;
	}

	void operator -=(const vec2& v) {
		x -= v.x;
		y -= v.y;
	}

	void operator *=(float scalar) {
		x *= scalar;
		y *= scalar;
	}

	inline float sqrDistance(const vec2& other) const {
		const vec2 d(x - other.x, y - other.y);
		return d.x * d.x + d.y * d.y;
	}

	float x;
	float y;
};

inline vec2 operator +(const vec2& a, const vec2& b) {
	return vec2(a.x + b.x, a.y + b.y);
}

inline vec2 operator -(const vec2& a, const vec2& b) {
	return vec2(a.x - b.x, a.y - b.y);
}

inline vec2 operator *(float scalar, const vec2& a) {
	return vec2(scalar * a.x, scalar * a.y);
}

static const vec2 vec2_zero;
