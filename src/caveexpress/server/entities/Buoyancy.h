#pragma once

/*
 * Author: Chris Campbell - www.iforce2d.net
 *
 * Copyright (c) 2006-2011 Erin Catto http://www.box2d.org
 *
 * This software is provided 'as-is', without any express or implied
 * warranty.  In no event will the authors be held liable for any damages
 * arising from the use of this software.
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 * 1. The origin of this software must not be misrepresented; you must not
 * claim that you wrote the original software. If you use this software
 * in a product, an acknowledgment in the product documentation would be
 * appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 * misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 */

// This file contains the support functions for finding the intersecting portion
// of two polygon fixtures.

#include "physics/Physics.h"
#include <vector>

namespace Buoyancy {

PhysicsVec2 computeCentroid (const std::vector<PhysicsVec2>& vs, float& area)
{
	const int count = (int) vs.size();
	assert(count >= 3);

	PhysicsVec2 c(0.0f, 0.0f);
	area = 0.0f;

	// pRef is the reference point for forming triangles.
	// It's location doesn't change the result (except for rounding error).
	PhysicsVec2 pRef(0.0f, 0.0f);

	const float inv3 = 1.0f / 3.0f;

	for (int i = 0; i < count; ++i) {
		// Triangle vertices.
		const PhysicsVec2 p1 = pRef;
		const PhysicsVec2 p2 = vs[i];
		const PhysicsVec2 p3 = i + 1 < count ? vs[i + 1] : vs[0];

		const PhysicsVec2 e1 = p2 - p1;
		const PhysicsVec2 e2 = p3 - p1;

		const float D = physCross(e1, e2);

		const float triangleArea = 0.5f * D;
		area += triangleArea;

		// Area weighted centroid
		c += triangleArea * inv3 * (p1 + p2 + p3);
	}

	// Centroid
	if (area > PhysicsEpsilon)
		c *= 1.0f / area;
	else
		area = 0;
	return c;
}

static inline bool inside (const PhysicsVec2& cp1, const PhysicsVec2& cp2, const PhysicsVec2& p)
{
	return (cp2.x - cp1.x) * (p.y - cp1.y) > (cp2.y - cp1.y) * (p.x - cp1.x);
}

static inline PhysicsVec2 intersection (const PhysicsVec2& cp1, const PhysicsVec2& cp2, const PhysicsVec2& s, const PhysicsVec2& e)
{
	const PhysicsVec2 dc(cp1.x - cp2.x, cp1.y - cp2.y);
	const PhysicsVec2 dp(s.x - e.x, s.y - e.y);
	const float n1 = cp1.x * cp2.y - cp1.y * cp2.x;
	const float n2 = s.x * e.y - s.y * e.x;
	const float n3 = 1.0f / (dc.x * dp.y - dc.y * dp.x);
	return PhysicsVec2((n1 * dp.x - n2 * dc.x) * n3, (n1 * dp.y - n2 * dc.y) * n3);
}

// http://rosettacode.org/wiki/Sutherland-Hodgman_polygon_clipping#JavaScript
bool findIntersectionOfFixtures (const PhysicsFixture fA, const PhysicsFixture fB, std::vector<PhysicsVec2>& outputVertices)
{
	// currently this only handles polygon vs polygon
	if (fA.getShapeType() != PhysicsShapeType::Polygon
			|| fB.getShapeType() != PhysicsShapeType::Polygon)
		return false;

	// fill 'subject polygon' from fixtureA polygon
	const int polyAVertexCount = fA.getPolygonVertexCount();
	for (int i = 0; i < polyAVertexCount; ++i)
		outputVertices.push_back(fA.getBody().getWorldPoint(fA.getPolygonVertex(i)));

	// fill 'clip polygon' from fixtureB polygon
	std::vector<PhysicsVec2> clipPolygon;
	const int polyBVertexCount = fB.getPolygonVertexCount();
	for (int i = 0; i < polyBVertexCount; i++)
		clipPolygon.push_back(fB.getBody().getWorldPoint(fB.getPolygonVertex(i)));

	PhysicsVec2 cp1 = clipPolygon[clipPolygon.size() - 1];
	const int clipPolygonSize = (int) clipPolygon.size();
	for (int j = 0; j < clipPolygonSize; ++j) {
		const PhysicsVec2& cp2 = clipPolygon[j];
		if (outputVertices.empty())
			return false;
		std::vector<PhysicsVec2> inputList = outputVertices;
		outputVertices.clear();
		PhysicsVec2 s = inputList.back(); // last on the input list
		const int inputListSize = (int) inputList.size();
		for (int i = 0; i < inputListSize; ++i) {
			const PhysicsVec2& e = inputList[i];
			if (inside(cp1, cp2, e)) {
				if (!inside(cp1, cp2, s)) {
					outputVertices.push_back(intersection(cp1, cp2, s, e));
				}
				outputVertices.push_back(e);
			} else if (inside(cp1, cp2, s)) {
				outputVertices.push_back(intersection(cp1, cp2, s, e));
			}
			s = e;
		}
		cp1 = cp2;
	}

	return !outputVertices.empty();
}

}
