#include "BezierCurve.h"
#include "engine/common/Math.h"
#include <assert.h>

BezierCurve::BezierCurve ()
{
}

BezierCurve::~BezierCurve ()
{
}

float BezierCurve::lerp (const double a, const double b, const double t)
{
	const double p = a + (b - a) * t;
	return p;
}

float BezierCurve::bezier (const Points& points, const double t)
{
	if (points.size() == 2) {
		const double p = lerp(points[0], points[1], t);
		return p;
	}

	assert(points.size() > 2);

	Points::const_iterator i = points.begin();
	double prev = *i++;
	Points lerped;
	for (; i != points.end(); ++i) {
		const double current = *i;
		const double p = lerp(prev, current, t);
		prev = current;
		lerped.push_back(p);
	}

	return bezier(lerped, t);
}
