#pragma once

#include <vector>

typedef std::vector<double> Points;

class BezierCurve {
private:
	// simple linear interpolation between two points
	static inline float lerp (const double a, const double b, const double t);

public:
	BezierCurve ();
	virtual ~BezierCurve ();

	// evaluate a point on a bezier-curve. t goes from 0 to 1.0
	static float bezier (const Points& points, const double t);
};
