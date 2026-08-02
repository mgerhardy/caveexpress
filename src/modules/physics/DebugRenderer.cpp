#include "DebugRenderer.h"
#include "common/ConfigManager.h"
#include "common/EventHandler.h"
#include "common/IFrontend.h"
#include <math.h>
#include <algorithm>

static int vx[2048];
static int vy[2048];

#define VX(val) (((val)) * _data.scale + _data.x)
#define VY(val) (((val)) * _data.scale + _data.y)

DebugRenderer::DebugRenderer (int pointCount, const ContactPoint *points, int traceCount, const TraceData *traceData, const std::vector<PhysicsVec2>& waterIntersectionPoints, const DebugRendererData& data, IFrontend *frontend) :
		_pointCount(pointCount), _points(points), _waterIntersectionPoints(waterIntersectionPoints), _traceCount(traceCount), _traceData(
				traceData), _data(data), _frontend(frontend)
{
}

DebugRenderer::~DebugRenderer ()
{
	const float k_impulseScale = 0.1f;
	const float k_axisScale = 0.3f;

	const bool drawFrictionImpulse = Config.getConfigVar("box2d_frictionnormals", "true")->getBoolValue();
	const bool drawContactNormals = Config.getConfigVar("box2d_contactnormals", "true")->getBoolValue();
	const bool drawContactImpulse = Config.getConfigVar("box2d_contactimpulse", "true")->getBoolValue();

	for (int i = 0; i < _pointCount; ++i) {
		const ContactPoint* point = &_points[i];

		if (point->state == PhysicsPointState::Add) {
			drawPoint(point->position, 0.2f, PhysicsColor(0.3f, 0.95f, 0.3f));
		} else if (point->state == PhysicsPointState::Persist) {
			drawPoint(point->position, 0.1f, PhysicsColor(0.3f, 0.3f, 0.95f));
		}

		if (drawContactNormals) {
			const PhysicsVec2& p1 = point->position;
			const PhysicsVec2 p2 = p1 + k_axisScale * point->normal;
			drawSegment(p1, p2, PhysicsColor(0.9f, 0.9f, 0.9f));
		}

		if (drawContactImpulse) {
			const PhysicsVec2& p1 = point->position;
			const PhysicsVec2 p2 = p1 + k_impulseScale * point->normalImpulse * point->normal;
			drawSegmentWithAlpha(p1, p2, PhysicsColor(0.9f, 0.9f, 0.3f), 0.5f);
		}

		if (drawFrictionImpulse) {
			const PhysicsVec2 tangent = physCross(point->normal, 1.0f);
			const PhysicsVec2& p1 = point->position;
			const PhysicsVec2 p2 = p1 + k_impulseScale * point->tangentImpulse * tangent;
			drawSegment(p1, p2, PhysicsColor(0.9f, 0.9f, 0.3f));
		}
	}

	for (int i = 0; i < _traceCount; ++i) {
		const TraceData* data = &_traceData[i];
		drawSegment(data->start, data->end, PhysicsColor(0.9f, 0.0f, 0.0f));
		drawPoint(data->start, 0.2f, PhysicsColor(0.0f, 0.95f, 0.3f));
		drawPoint(data->end, 0.2f, PhysicsColor(0.3f, 0.7f, 0.0f));
	}

	if (!_waterIntersectionPoints.empty())
		drawPolygon(&_waterIntersectionPoints[0], (int)_waterIntersectionPoints.size(), PhysicsColor(0.0f, 1.0f, 1.0f));
}

void DebugRenderer::drawPolygon (const PhysicsVec2* vertices, int vertexCount, const PhysicsColor& color)
{
	for (int i = 0; i < vertexCount; i += 2) {
		if (i + 1 < vertexCount)
			drawSegment(vertices[i], vertices[i + 1], color);
	}
}

void DebugRenderer::drawSolidPolygon (const PhysicsVec2* vertices, int vertexCount, const PhysicsColor& color)
{
	const Color rgba = { color.r, color.g, color.b, 0.5f };
	for (int i = 0; i < vertexCount; ++i) {
		vx[i] = VX(vertices[i].x);
		vy[i] = VY(vertices[i].y);
	}
	_frontend->renderFilledPolygon(vx, vy, vertexCount, rgba);
	drawPolygon(vertices, vertexCount, color);
}

void DebugRenderer::drawCircle(const PhysicsVec2 &center, float radius, const PhysicsColor &color)
{
	const float k_segments = 16.0f;
	const float k_increment = 2.0f * (float)M_PI / k_segments;
	float sinInc = sinf(k_increment);
	float cosInc = cosf(k_increment);
	PhysicsVec2 r1(1.0f, 0.0f);
	PhysicsVec2 v1 = center + radius * r1;

	for (int i = 0; i < (int)k_segments; ++i)
	{
		PhysicsVec2 r2;
		r2.x = cosInc * r1.x - sinInc * r1.y;
		r2.y = sinInc * r1.x + cosInc * r1.y;
		PhysicsVec2 v2 = center + radius * r2;
		drawSegment(v1, v2, color);
		r1 = r2;
		v1 = v2;
	}
}

void DebugRenderer::drawSolidCircle (const PhysicsVec2& center, float radius, const PhysicsVec2& axis, const PhysicsColor& color)
{
	const float k_segments = 16.0f;
	const float k_increment = 2.0f * (float)M_PI / k_segments;
	float sinInc = sinf(k_increment);
	float cosInc = cosf(k_increment);
	PhysicsVec2 r1(1.0f, 0.0f);
	PhysicsVec2 v1 = center + radius * r1;
	for (int i = 0; i < (int)k_segments; ++i)
	{
		PhysicsVec2 r2;
		r2.x = cosInc * r1.x - sinInc * r1.y;
		r2.y = sinInc * r1.x + cosInc * r1.y;
		PhysicsVec2 v2 = center + radius * r2;
		drawSegment(v1, v2, color);
		r1 = r2;
		v1 = v2;
	}

	PhysicsVec2 p = center + radius * axis;
	drawSegment(center, p, color);
}

void DebugRenderer::drawPoint (const PhysicsVec2& p, float size, const PhysicsColor& color)
{
	size = std::max(size, 1.0f);
	const Color rgba = { color.r, color.g, color.b, 0.5f };
	const int minx = (int)(VX(p.x) - size / 2.0f);
	const int miny = (int)(VY(p.y) - size / 2.0f);
	_frontend->renderFilledRect(minx, miny, (int)size, (int)size, rgba);
}

void DebugRenderer::drawSegment (const PhysicsVec2& p1, const PhysicsVec2& p2, const PhysicsColor& color)
{
	drawSegmentWithAlpha(p1, p2, color, 1.0f);
}

void DebugRenderer::drawSegmentWithAlpha (const PhysicsVec2& p1, const PhysicsVec2& p2, const PhysicsColor& color, float alpha)
{
	const Color rgba = { color.r, color.g, color.b, alpha };
	const int p1x = VX(p1.x);
	const int p1y = VY(p1.y);
	const int p2x = VX(p2.x);
	const int p2y = VY(p2.y);
	_frontend->renderLine(p1x, p1y, p2x, p2y, rgba);
}

void DebugRenderer::drawTransform (const PhysicsTransform& xf)
{
	const PhysicsVec2& p1 = xf.p;
	const float k_axisScale = 0.4f;

	const PhysicsVec2 p2 = p1 + k_axisScale * xf.getXAxis();
	drawSegment(p1, p2, PhysicsColor(1.0f, 0.0f, 0.0f));

	const PhysicsVec2 p3 = p1 + k_axisScale * xf.getYAxis();
	drawSegment(p1, p3, PhysicsColor(0.0f, 1.0f, 0.0f));
}
