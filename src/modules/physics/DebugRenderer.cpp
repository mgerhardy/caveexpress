#include "DebugRenderer.h"
#include "common/ConfigManager.h"
#include "common/EventHandler.h"
#include "common/IFrontend.h"
#include "common/ConfigManager.h"
#include <math.h>

static int vx[2048];
static int vy[2048];

#define VX(val) (((val)) * _data.scale + _data.x)
#define VY(val) (((val)) * _data.scale + _data.y)

DebugRenderer::DebugRenderer (int pointCount, const ContactPoint *points, int traceCount, const TraceData *traceData, const std::vector<b2Vec2>& waterIntersectionPoints, const DebugRendererData& data, IFrontend *frontend) :
		b2Draw(), _pointCount(pointCount), _points(points), _waterIntersectionPoints(waterIntersectionPoints), _traceCount(traceCount), _traceData(
				traceData), _data(data), _frontend(frontend)
{
	SetFlags(b2Draw::e_shapeBit | b2Draw::e_jointBit | b2Draw::e_aabbBit | b2Draw::e_pairBit
			| b2Draw::e_centerOfMassBit);
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

		if (point->state == b2_addState) {
			DrawPoint(point->position, 0.2f, b2Color(0.3f, 0.95f, 0.3f));
		} else if (point->state == b2_persistState) {
			DrawPoint(point->position, 0.1f, b2Color(0.3f, 0.3f, 0.95f));
		}

		if (drawContactNormals) {
			const b2Vec2& p1 = point->position;
			const b2Vec2 p2 = p1 + k_axisScale * point->normal;
			DrawSegment(p1, p2, b2Color(0.9f, 0.9f, 0.9f));
		}

		if (drawContactImpulse) {
			const b2Vec2& p1 = point->position;
			const b2Vec2 p2 = p1 + k_impulseScale * point->normalImpulse * point->normal;
			DrawSegmentWithAlpha(p1, p2, b2Color(0.9f, 0.9f, 0.3f), 0.5f);
		}

		if (drawFrictionImpulse) {
			const b2Vec2 tangent = b2Cross(point->normal, 1.0f);
			const b2Vec2& p1 = point->position;
			const b2Vec2 p2 = p1 + k_impulseScale * point->tangentImpulse * tangent;
			DrawSegment(p1, p2, b2Color(0.9f, 0.9f, 0.3f));
		}
	}
	for (int i = 0; i < _traceCount; ++i) {
		const TraceData* data = &_traceData[i];
		DrawSegment(data->start, data->end, b2Color(0.9f, 0.0f, 0.0f));
		DrawPoint(data->start, 0.2f, b2Color(0.0f, 0.95f, 0.3f));
		DrawPoint(data->end, 0.2f, b2Color(0.3f, 0.7f, 0.0f));
	}

	if (!_waterIntersectionPoints.empty())
		DrawPolygon(&_waterIntersectionPoints[0], _waterIntersectionPoints.size(), b2Color(0.0f, 1.0f, 1.0f));
}

void DebugRenderer::DrawPolygon (const b2Vec2* vertices, int vertexCount, const b2Color& color)
{
	for (int i = 0; i < vertexCount; i += 2) {
		DrawSegment(vertices[i], vertices[i + 1], color);
	}
}

void DebugRenderer::DrawSolidPolygon (const b2Vec2* vertices, int vertexCount, const b2Color& color)
{
	const Color rgba = { color.r, color.g, color.b, 0.5f };
	for (int i = 0; i < vertexCount; ++i) {
		vx[i] = VX(vertices[i].x);
		vy[i] = VY(vertices[i].y);
	}
	_frontend->renderFilledPolygon(vx, vy, vertexCount, rgba);
	DrawPolygon(vertices, vertexCount, color);
}

void DebugRenderer::DrawCircle (const b2Vec2& center, float radius, const b2Color& color)
{
	const float k_segments = 16.0f;
	const float k_increment = 2.0f * b2_pi / k_segments;
	float theta = 0.0f;

	for (int i = 0; i < k_segments; i += 2) {
		const b2Vec2 p1 = center + radius * b2Vec2(cosf(theta), sinf(theta));
		theta += k_increment;
		const b2Vec2 p2 = center + radius * b2Vec2(cosf(theta), sinf(theta));
		theta += k_increment;
		DrawSegment(p1, p2, color);
	}
}

void DebugRenderer::DrawSolidCircle (const b2Vec2& center, float radius, const b2Vec2& axis, const b2Color& color)
{
	const float k_segments = 16.0f;
	const int vertexCount = 16;
	const float k_increment = 2.0f * b2_pi / k_segments;
	float theta = 0.0f;

	b2Vec2 vertices[vertexCount];
	for (int i = 0; i < k_segments; ++i) {
		const b2Vec2 v = center + radius * b2Vec2(cosf(theta), sinf(theta));
		vertices[i] = v;
		theta += k_increment;
	}

	DrawSolidPolygon(vertices, vertexCount, color);

	// Draw the axis line
	DrawSegment(center, center + radius * axis, color);
}

void DebugRenderer::DrawPoint (const b2Vec2& p, float size, const b2Color& color)
{
	const Color rgba = { color.r, color.g, color.b, 0.5f };
	const float minx = VX(p.x - size / 2.0f);
	const float maxx = VX(p.x + size / 2.0f);
	const float miny = VY(p.y - size / 2.0f);
	const float maxy = VY(p.y + size / 2.0f);
	_frontend->renderFilledRect(minx, miny, maxx - minx, maxy - miny, rgba);
}

void DebugRenderer::DrawSegment (const b2Vec2& p1, const b2Vec2& p2, const b2Color& color)
{
	DrawSegmentWithAlpha(p1, p2, color, 1.0f);
}

void DebugRenderer::DrawSegmentWithAlpha (const b2Vec2& p1, const b2Vec2& p2, const b2Color& color, float alpha)
{
	const Color rgba = { color.r, color.g, color.b, alpha };
	const int p1x = VX(p1.x);
	const int p1y = VY(p1.y);
	const int p2x = VX(p2.x);
	const int p2y = VY(p2.y);
	_frontend->renderLine(p1x, p1y, p2x, p2y, rgba);
}

void DebugRenderer::DrawTransform (const b2Transform& xf)
{
	const b2Vec2& p1 = xf.p;
	const float k_axisScale = 0.4f;

	const b2Vec2 p2 = p1 + k_axisScale * xf.q.GetXAxis();
	DrawSegment(p1, p2, b2Color(1.0f, 0.0f, 0.0f));

	const b2Vec2 p3 = p1 + k_axisScale * xf.q.GetYAxis();
	DrawSegment(p1, p3, b2Color(0.0f, 1.0f, 0.0f));
}
