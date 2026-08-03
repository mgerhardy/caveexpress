#include "SpriteShapeTraits.h"
#include "caveexpress/shared/CaveExpressSpriteType.h"
#include "common/Math.h"
#include <algorithm>
#include <cmath>
#include <vector>

namespace caveexpress {

namespace {

const float EDGE = 0.5f;
const float EDGE_EPS = 0.08f;

struct ShapeBounds {
	float minX = EDGE;
	float maxX = -EDGE;
	float minY = EDGE;
	float maxY = -EDGE;
	float sumX = 0.0f;
	float sumY = 0.0f;
	int vertCount = 0;
	float polyArea = 0.0f;
	bool valid = false;

	float width () const { return maxX - minX; }
	float height () const { return maxY - minY; }
	float centroidX () const { return vertCount > 0 ? sumX / static_cast<float>(vertCount) : 0.0f; }
	float centroidY () const { return vertCount > 0 ? sumY / static_cast<float>(vertCount) : 0.0f; }

	bool touchesTop () const { return maxY >= EDGE - EDGE_EPS; }
	bool touchesBottom () const { return minY <= -EDGE + EDGE_EPS; }
	bool touchesLeft () const { return minX <= -EDGE + EDGE_EPS; }
	bool touchesRight () const { return maxX >= EDGE - EDGE_EPS; }
};

float shoelaceArea (const std::vector<SpriteVertex>& verts)
{
	if (verts.size() < 3)
		return 0.0f;
	float area = 0.0f;
	for (size_t i = 0; i < verts.size(); ++i) {
		const SpriteVertex& a = verts[i];
		const SpriteVertex& b = verts[(i + 1) % verts.size()];
		area += a.x * b.y - b.x * a.y;
	}
	return std::fabs(area) * 0.5f;
}

ShapeBounds collectBounds (const SpriteDefPtr& def)
{
	ShapeBounds b;
	if (!def || !def->hasShape())
		return b;

	auto addVert = [&] (float x, float y) {
		b.minX = std::min(b.minX, x);
		b.maxX = std::max(b.maxX, x);
		b.minY = std::min(b.minY, y);
		b.maxY = std::max(b.maxY, y);
		b.sumX += x;
		b.sumY += y;
		++b.vertCount;
	};

	for (const SpritePolygon& poly : def->polygons) {
		b.polyArea += shoelaceArea(poly.vertices);
		for (const SpriteVertex& v : poly.vertices)
			addVert(v.x, v.y);
	}
	for (const SpriteCircle& c : def->circles) {
		b.polyArea += static_cast<float>(M_PI) * c.radius * c.radius;
		addVert(c.center.x - c.radius, c.center.y);
		addVert(c.center.x + c.radius, c.center.y);
		addVert(c.center.x, c.center.y - c.radius);
		addVert(c.center.x, c.center.y + c.radius);
	}

	b.valid = b.vertCount > 0;
	return b;
}

bool isUnitTile (const SpriteDefPtr& def)
{
	return def && def->width <= 1.01f && def->height <= 1.01f;
}

bool isHalfTriangleArea (float area)
{
	return area >= 0.35f && area <= 0.65f;
}

}

SpriteShapeTraits analyzeSpriteShape (const SpriteDefPtr& def)
{
	SpriteShapeTraits t;
	if (!def || !isUnitTile(def))
		return t;

	const SpriteType& type = def->type;
	const bool isSlopeType = SpriteTypes::isSlope(type);
	const bool rockLike = SpriteTypes::isRock(type) || isSlopeType;
	const bool groundLike = SpriteTypes::isGround(type);

	if (!def->hasShape()) {
		// Default tile body is a full AABB box. Slopes always have polygons in Lua.
		if (SpriteTypes::isRock(type))
			t.fullSolid = true;
		return t;
	}

	const ShapeBounds b = collectBounds(def);
	if (!b.valid)
		return t;

	const float area = b.polyArea;

	// Walkable slopes (*-01): Lua tags slope-left/right; bottom-biased half triangles.
	if (SpriteTypes::isSlopeLeft(type)) {
		t.slopeL = true;
		return t;
	}
	if (SpriteTypes::isSlopeRight(type)) {
		t.slopeR = true;
		return t;
	}
	// Shape fallback if a theme ever omits SpriteType but keeps the ramp polygon.
	if (rockLike && !isSlopeType && isHalfTriangleArea(area) && b.height() >= 0.70f
			&& b.touchesBottom() && b.centroidY() < -0.05f) {
		if (b.centroidX() > 0.05f) {
			t.slopeL = true;
			return t;
		}
		if (b.centroidX() < -0.05f) {
			t.slopeR = true;
			return t;
		}
	}

	if (rockLike && area >= 0.85f && b.touchesTop() && b.touchesBottom() && b.touchesLeft() && b.touchesRight()) {
		t.fullSolid = true;
		return t;
	}

	// Thin top slab (hanging decks): short AABB along the top edge.
	if (groundLike && b.height() <= 0.45f && b.touchesTop() && b.minY > -0.15f) {
		t.thinTopSlab = true;
		return t;
	}

	// Shim: small peak hanging from the top edge into the tile.
	if (SpriteTypes::isRock(type) && area <= 0.40f && b.touchesTop() && !b.touchesBottom() && b.height() <= 0.60f
			&& b.width() >= 0.70f && std::fabs(b.centroidX()) <= 0.20f) {
		t.shim = true;
		return t;
	}

	// Undercuts (*-02): rock half-triangles biased toward the top (not walkable slopes).
	if (SpriteTypes::isRock(type) && isHalfTriangleArea(area) && b.height() >= 0.70f
			&& b.touchesTop() && b.centroidY() > 0.05f) {
		if (b.centroidX() > 0.05f) {
			t.undercutL = true;
			return t;
		}
		if (b.centroidX() < -0.05f) {
			t.undercutR = true;
			return t;
		}
	}

	return t;
}

}
