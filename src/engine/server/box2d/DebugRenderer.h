#pragma once

#include "Box2D/Box2D.h"
#include "engine/common/Compiler.h"
#include "engine/common/DebugRendererData.h"
#include <vector>

// forward decl
class IFrontend;

struct ContactPoint {
	b2Fixture* fixtureA;
	b2Fixture* fixtureB;
	b2Vec2 normal;
	b2Vec2 position;
	b2PointState state;
	float32 normalImpulse;
	float32 tangentImpulse;
};

struct TraceData {
	b2Vec2 start;
	b2Vec2 end;
	float fraction;
};

#define DEBUG_RENDERER_MAX_COLORS 128
class DebugRenderer: public b2Draw {
private:
	int _activeProgram;
	bool _enableTextureArray;
	float _colorArray[DEBUG_RENDERER_MAX_COLORS * 4];
	int _pointCount;
	const ContactPoint *_points;
	const std::vector<b2Vec2>& _waterIntersectionPoints;
	int _traceCount;
	const TraceData *_traceData;

	void DrawPoint(const b2Vec2& p, float32 size, const b2Color& color);
	void DrawSegmentWithAlpha (const b2Vec2& p1, const b2Vec2& p2, const b2Color& color, float alpha);
	void setColorPointer (const b2Color& color, float alpha, int amount);
public:
	DebugRenderer (int pointCount, const ContactPoint *points, int traceCount, const TraceData *traceData, const std::vector<b2Vec2>& waterIntersectionPoints, const DebugRendererData& rect);
	virtual ~DebugRenderer ();

	// b2Draw
	void DrawPolygon (const b2Vec2* vertices, int32 vertexCount, const b2Color& color) override;
	void DrawSolidPolygon (const b2Vec2* vertices, int32 vertexCount, const b2Color& color) override;
	void DrawCircle (const b2Vec2& center, float32 radius, const b2Color& color) override;
	void DrawSolidCircle (const b2Vec2& center, float32 radius, const b2Vec2& axis, const b2Color& color) override;
	void DrawSegment (const b2Vec2& p1, const b2Vec2& p2, const b2Color& color) override;
	void DrawTransform (const b2Transform& xf);
};

