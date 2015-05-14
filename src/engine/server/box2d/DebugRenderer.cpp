#include "DebugRenderer.h"
#include "engine/common/ConfigManager.h"
#include "engine/common/EventHandler.h"
#include "engine/client/GLShared.h"
#include "engine/common/IFrontend.h"
#include <math.h>
#include <SDL_platform.h>

#ifndef SDL_VIDEO_OPENGL
#define NO_DEBUG_RENDERER
#endif

DebugRenderer::DebugRenderer (int pointCount, const ContactPoint *points, int traceCount, const TraceData *traceData, const std::vector<b2Vec2>& waterIntersectionPoints, const DebugRendererData& data) :
		b2Draw(), _activeProgram(0), _pointCount(pointCount), _points(points), _traceCount(traceCount), _traceData(
				traceData), _waterIntersectionPoints(waterIntersectionPoints), _enableTextureArray(false)
{
	memset(_colorArray, 0, sizeof(_colorArray));
#ifndef NO_DEBUG_RENDERER
	if (GLContext::get().areShadersSupported()) {
		glGetIntegerv(GL_ACTIVE_PROGRAM, &_activeProgram);
		const GLenum glError = glGetError();
		if (glError != GL_INVALID_ENUM)
			GLContext::get().ctx_glUseProgram(0);
		else
			_activeProgram = 0;
	}
	SetFlags(b2Draw::e_shapeBit | b2Draw::e_jointBit | b2Draw::e_aabbBit | b2Draw::e_pairBit
			| b2Draw::e_centerOfMassBit);
	GL_checkError();
	glPushMatrix();
	const float scale = static_cast<float>(data.scale);
	const float x = static_cast<float>(data.x / scale);
	const float y = static_cast<float>(data.y / scale);

	glScalef(scale, scale, 1.0f);
	glTranslatef(x, y, 0.0f);
	glDisable(GL_TEXTURE_2D);

	_enableTextureArray = glIsEnabled(GL_TEXTURE_COORD_ARRAY);
	if (_enableTextureArray)
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);

	GL_checkError();
#endif
}

DebugRenderer::~DebugRenderer ()
{
#ifndef NO_DEBUG_RENDERER
	const float32 k_impulseScale = 0.1f;
	const float32 k_axisScale = 0.3f;

	const bool drawFrictionImpulse = true;
	const bool drawContactNormals = true;
	const bool drawContactImpulse = true;

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

	if (_enableTextureArray)
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glEnable(GL_TEXTURE_2D);
	glPopMatrix();
	GL_checkError();
	if (_activeProgram != 0)
		GLContext::get().ctx_glUseProgram(_activeProgram);
#endif
}

void DebugRenderer::setColorPointer (const b2Color& color, float alpha, int amount)
{
#ifndef NO_DEBUG_RENDERER
	if (amount > DEBUG_RENDERER_MAX_COLORS)
		amount = DEBUG_RENDERER_MAX_COLORS;
	for (int i = 0; i < amount; ++i) {
		_colorArray[i * 4 + 0] = color.r;
		_colorArray[i * 4 + 1] = color.g;
		_colorArray[i * 4 + 2] = color.b;
		_colorArray[i * 4 + 3] = alpha;
	}
	glColorPointer(4, GL_FLOAT, 0, _colorArray);
#endif
}

void DebugRenderer::DrawPolygon (const b2Vec2* vertices, int vertexCount, const b2Color& color)
{
#ifndef NO_DEBUG_RENDERER
	setColorPointer(color, 1.0f, vertexCount);
	glVertexPointer(2, GL_FLOAT, 0, vertices);
	glDrawArrays(GL_LINE_LOOP, 0, vertexCount);
	GL_checkError();
#endif
}

void DebugRenderer::DrawSolidPolygon (const b2Vec2* vertices, int vertexCount, const b2Color& color)
{
#ifndef NO_DEBUG_RENDERER
	glVertexPointer(2, GL_FLOAT, 0, vertices);

	setColorPointer(color, 0.5f, vertexCount);
	glDrawArrays(GL_TRIANGLE_FAN, 0, vertexCount);

	setColorPointer(color, 1.0f, vertexCount);
	glDrawArrays(GL_LINE_LOOP, 0, vertexCount);
	GL_checkError();
#endif
}

void DebugRenderer::DrawCircle (const b2Vec2& center, float32 radius, const b2Color& color)
{
#ifndef NO_DEBUG_RENDERER
	const float32 k_segments = 16.0f;
	const int vertexCount = 16;
	const float32 k_increment = 2.0f * b2_pi / k_segments;
	float32 theta = 0.0f;

	GLfloat glVertices[vertexCount * 2];
	for (int i = 0; i < k_segments; ++i) {
		const b2Vec2 v = center + radius * b2Vec2(cosf(theta), sinf(theta));
		glVertices[i * 2] = v.x;
		glVertices[i * 2 + 1] = v.y;
		theta += k_increment;
	}

	setColorPointer(color, 1.0f, vertexCount);
	glVertexPointer(2, GL_FLOAT, 0, glVertices);
	glDrawArrays(GL_TRIANGLE_FAN, 0, vertexCount);
	GL_checkError();
#endif
}

void DebugRenderer::DrawSolidCircle (const b2Vec2& center, float32 radius, const b2Vec2& axis, const b2Color& color)
{
#ifndef NO_DEBUG_RENDERER
	const float32 k_segments = 16.0f;
	const int vertexCount = 16;
	const float32 k_increment = 2.0f * b2_pi / k_segments;
	float32 theta = 0.0f;

	GLfloat glVertices[vertexCount * 2];
	for (int i = 0; i < k_segments; ++i) {
		const b2Vec2 v = center + radius * b2Vec2(cosf(theta), sinf(theta));
		glVertices[i * 2] = v.x;
		glVertices[i * 2 + 1] = v.y;
		theta += k_increment;
	}

	setColorPointer(color, 0.5f, vertexCount);
	glVertexPointer(2, GL_FLOAT, 0, glVertices);
	glDrawArrays(GL_TRIANGLE_FAN, 0, vertexCount);
	setColorPointer(color, 1.0f, vertexCount);
	glDrawArrays(GL_LINE_LOOP, 0, vertexCount);
	GL_checkError();

	// Draw the axis line
	DrawSegment(center, center + radius * axis, color);
#endif
}

void DebugRenderer::DrawPoint (const b2Vec2& p, float32 size, const b2Color& color)
{
#ifndef NO_DEBUG_RENDERER
	setColorPointer(color, 1.0f, 4);
	const float minx = p.x - size / 2.0f;
	const float maxx = p.x + size / 2.0f;
	const float miny = p.y - size / 2.0f;
	const float maxy = p.y + size / 2.0f;
	const float vertices[] = { minx, miny, maxx, miny, minx, maxy, maxx, maxy };
	glVertexPointer(2, GL_FLOAT, 0, vertices);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	GL_checkError();
#endif
}

void DebugRenderer::DrawSegment (const b2Vec2& p1, const b2Vec2& p2, const b2Color& color)
{
	DrawSegmentWithAlpha(p1, p2, color, 1.0f);
}

void DebugRenderer::DrawSegmentWithAlpha (const b2Vec2& p1, const b2Vec2& p2, const b2Color& color, float alpha)
{
#ifndef NO_DEBUG_RENDERER
	setColorPointer(color, alpha, 2);
	const GLfloat glVertices[] = { p1.x, p1.y, p2.x, p2.y };
	glVertexPointer(2, GL_FLOAT, 0, glVertices);
	glDrawArrays(GL_LINES, 0, 2);
	GL_checkError();
#endif
}

void DebugRenderer::DrawTransform (const b2Transform& xf)
{
	const b2Vec2& p1 = xf.p;
	const float32 k_axisScale = 0.4f;

	const b2Vec2 p2 = p1 + k_axisScale * xf.q.GetXAxis();
	DrawSegment(p1, p2, b2Color(1.0f, 0.0f, 0.0f));

	const b2Vec2 p3 = p1 + k_axisScale * xf.q.GetYAxis();
	DrawSegment(p1, p3, b2Color(0.0f, 1.0f, 0.0f));
}
