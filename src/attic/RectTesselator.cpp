#include "RectTesselator.h"

RectTesselator::RectTesselator (uint32_t tesselation)
{
	static const glm::vec2 uv0(0.0f, 1.0f);
	static const glm::vec2 uv1(1.0f, 0.0f);
	static const glm::vec2 uv2(0.0f, 0.0f);
	static const glm::vec2 meshBounds(uv1.x - uv0.x, uv2.y - uv0.y);
	static const glm::vec2 uvBounds(uv1.x - uv0.y, uv0.y - uv2.y);
	static const glm::vec2 uvPos = uv2;
	static const glm::vec2 anchorOffset(meshBounds.x / 2, meshBounds.y / 2);

	const uint32_t strucWidth = tesselation + 2;
	const float segmentWidth = 1.0f / (tesselation + 1);
	const float scaleX = meshBounds.x / (tesselation + 1);
	const float scaleY = meshBounds.y / (tesselation + 1);

	for (float y = 0.0f; y < strucWidth; ++y) {
		for (float x = 0.0f; x < strucWidth; ++x) {
			const glm::vec2 v = glm::vec2(x * scaleX - anchorOffset.x, y * scaleY - anchorOffset.y);
			const glm::vec2 t = glm::vec2((x * segmentWidth * uvBounds.x) + uvPos.x,
					uvBounds.y - (y * segmentWidth * uvBounds.y) + uvPos.y);
			_vertices.push_back(v);
			_texcoords.push_back(t);
		}
	}

	for (int y = 0; y < tesselation + 1; ++y) {
		for (int x = 0; x < tesselation + 1; ++x) {
			_indices.push_back((y * strucWidth) + x);
			_indices.push_back(((y + 1) * strucWidth) + x);
			_indices.push_back((y * strucWidth) + x + 1);
			_indices.push_back(((y + 1) * strucWidth) + x);
			_indices.push_back(((y + 1) * strucWidth) + x + 1);
			_indices.push_back((y * strucWidth) + x + 1);
		}
	}
}

RectTesselator::~RectTesselator ()
{
}
