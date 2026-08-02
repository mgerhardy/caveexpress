#pragma once

#include "physics/Physics.h"
#include "common/IFrontend.h"
#include "common/Compiler.h"
#include "common/DebugRendererData.h"
#include <vector>

class IFrontend;

struct ContactPoint {
	PhysicsFixture fixtureA;
	PhysicsFixture fixtureB;
	PhysicsVec2 normal;
	PhysicsVec2 position;
	PhysicsPointState state;
	float normalImpulse;
	float tangentImpulse;
};

struct TraceData {
	PhysicsVec2 start;
	PhysicsVec2 end;
	float fraction;
};

#define DEBUG_RENDERER_MAX_COLORS 128
class DebugRenderer: public IPhysicsDebugDraw {
private:
	int _pointCount;
	const ContactPoint *_points;
	const std::vector<PhysicsVec2>& _waterIntersectionPoints;
	int _traceCount;
	const TraceData *_traceData;
	const DebugRendererData _data;
	IFrontend* _frontend;

	void drawSegmentWithAlpha (const PhysicsVec2& p1, const PhysicsVec2& p2, const PhysicsColor& color, float alpha);
public:
	DebugRenderer (int pointCount, const ContactPoint *points, int traceCount, const TraceData *traceData, const std::vector<PhysicsVec2>& waterIntersectionPoints, const DebugRendererData& rect, IFrontend* frontend);
	virtual ~DebugRenderer ();

	void drawPolygon (const PhysicsVec2* vertices, int vertexCount, const PhysicsColor& color) override;
	void drawSolidPolygon (const PhysicsVec2* vertices, int vertexCount, const PhysicsColor& color) override;
	void drawCircle (const PhysicsVec2& center, float radius, const PhysicsColor& color) override;
	void drawSolidCircle (const PhysicsVec2& center, float radius, const PhysicsVec2& axis, const PhysicsColor& color) override;
	void drawSegment (const PhysicsVec2& p1, const PhysicsVec2& p2, const PhysicsColor& color) override;
	void drawTransform (const PhysicsTransform& xf) override;
	void drawPoint (const PhysicsVec2& p, float size, const PhysicsColor& color) override;
};
