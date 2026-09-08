#pragma once

#include "imgui.h"
#include "common/IFrontend.h"
#include "common/Math.h"
#include "textures/Texture.h"
#include "textures/TextureCoords.h"
#include "sprites/Sprite.h"
#include "common/Layer.h"
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <vector>

inline ImTextureID mapEditorToImTextureID (IFrontend* frontend, const Texture* texture)
{
	return static_cast<ImTextureID>(reinterpret_cast<uintptr_t>(frontend->getTextureData(texture)));
}

struct MapEditorSpriteQuad {
	ImTextureID id = 0;
	ImVec2 pMin;
	ImVec2 pMax;
	ImVec2 uv0;
	ImVec2 uv1;
	ImVec2 uv2;
	ImVec2 uv3;
	ImU32 col = IM_COL32_WHITE;
	int16_t angle = 0;
	uint8_t spriteLayer = 0;
	uint8_t mapLayer = 0;
};

inline ImU32 mapEditorColorWithAlpha (ImU32 col, float alpha)
{
	const int a = static_cast<int>(((col >> IM_COL32_A_SHIFT) & 0xFF) * alpha);
	return (col & ~IM_COL32_A_MASK) | (static_cast<ImU32>(a) << IM_COL32_A_SHIFT);
}

inline void mapEditorAddTexture (ImDrawList* drawList, IFrontend* frontend, Texture* texture,
		const ImVec2& pMin, const ImVec2& pMax, float alpha = 1.0f, int16_t angle = 0)
{
	if (drawList == nullptr || frontend == nullptr || texture == nullptr || !texture->isValid())
		return;

	const TextureCoords coords(texture);
	const ImVec2 uv0(coords.texCoords[0], coords.texCoords[1]);
	const ImVec2 uv1(coords.texCoords[2], coords.texCoords[3]);
	const ImVec2 uv2(coords.texCoords[4], coords.texCoords[5]);
	const ImVec2 uv3(coords.texCoords[6], coords.texCoords[7]);
	const ImU32 col = mapEditorColorWithAlpha(IM_COL32_WHITE, alpha);
	const ImTextureID id = mapEditorToImTextureID(frontend, texture);

	if (angle == 0) {
		drawList->AddImage(id, pMin, pMax, uv0, uv2, col);
		return;
	}

	const ImVec2 center((pMin.x + pMax.x) * 0.5f, (pMin.y + pMax.y) * 0.5f);
	const float hw = (pMax.x - pMin.x) * 0.5f;
	const float hh = (pMax.y - pMin.y) * 0.5f;
	const float rad = static_cast<float>(angle) * static_cast<float>(DEG2RAD);
	const float c = std::cos(rad);
	const float s = std::sin(rad);
	auto rot = [&] (float x, float y) -> ImVec2 {
		return ImVec2(center.x + x * c - y * s, center.y + x * s + y * c);
	};
	drawList->AddImageQuad(id, rot(-hw, -hh), rot(hw, -hh), rot(hw, hh), rot(-hw, hh), uv0, uv1, uv2, uv3, col);
}

inline void mapEditorEmitQuad (ImDrawList* drawList, const MapEditorSpriteQuad& q)
{
	if (q.angle == 0) {
		drawList->AddImage(q.id, q.pMin, q.pMax, q.uv0, q.uv2, q.col);
		return;
	}
	const ImVec2 center((q.pMin.x + q.pMax.x) * 0.5f, (q.pMin.y + q.pMax.y) * 0.5f);
	const float hw = (q.pMax.x - q.pMin.x) * 0.5f;
	const float hh = (q.pMax.y - q.pMin.y) * 0.5f;
	const float rad = static_cast<float>(q.angle) * static_cast<float>(DEG2RAD);
	const float c = std::cos(rad);
	const float s = std::sin(rad);
	auto rot = [&] (float x, float y) -> ImVec2 {
		return ImVec2(center.x + x * c - y * s, center.y + x * s + y * c);
	};
	drawList->AddImageQuad(q.id, rot(-hw, -hh), rot(hw, -hh), rot(hw, hh), rot(-hw, hh),
			q.uv0, q.uv1, q.uv2, q.uv3, q.col);
}

inline bool mapEditorCollectTexture (std::vector<MapEditorSpriteQuad>& quads, IFrontend* frontend,
		Texture* texture, const ImVec2& pMin, const ImVec2& pMax, float alpha = 1.0f, int16_t angle = 0,
		uint8_t spriteLayer = 0, uint8_t mapLayer = 0)
{
	if (frontend == nullptr || texture == nullptr || !texture->isValid())
		return false;

	const TextureCoords coords(texture);
	MapEditorSpriteQuad q;
	q.id = mapEditorToImTextureID(frontend, texture);
	q.pMin = pMin;
	q.pMax = pMax;
	q.uv0 = ImVec2(coords.texCoords[0], coords.texCoords[1]);
	q.uv1 = ImVec2(coords.texCoords[2], coords.texCoords[3]);
	q.uv2 = ImVec2(coords.texCoords[4], coords.texCoords[5]);
	q.uv3 = ImVec2(coords.texCoords[6], coords.texCoords[7]);
	q.col = mapEditorColorWithAlpha(IM_COL32_WHITE, alpha);
	q.angle = angle;
	q.spriteLayer = spriteLayer;
	q.mapLayer = mapLayer;
	quads.push_back(q);
	return true;
}

inline void mapEditorCollectSprite (std::vector<MapEditorSpriteQuad>& quads, IFrontend* frontend,
		const SpritePtr& sprite, const ImVec2& pMin, const ImVec2& pMax, float alpha = 1.0f,
		int16_t angle = 0, uint8_t mapLayer = 0)
{
	if (!sprite)
		return;
	for (Layer layer = LAYER_BACK; layer < MAX_LAYERS; ++layer) {
		const TexturePtr& texture = sprite->getActiveTexture(layer);
		if (!texture)
			continue;
		mapEditorCollectTexture(quads, frontend, texture.get(), pMin, pMax, alpha, angle,
				static_cast<uint8_t>(layer), mapLayer);
	}
}

inline void mapEditorFlushSprites (ImDrawList* drawList, std::vector<MapEditorSpriteQuad>& quads)
{
	if (drawList == nullptr || quads.empty())
		return;
	std::stable_sort(quads.begin(), quads.end(), [] (const MapEditorSpriteQuad& a, const MapEditorSpriteQuad& b) {
		if (a.mapLayer != b.mapLayer)
			return a.mapLayer < b.mapLayer;
		if (a.spriteLayer != b.spriteLayer)
			return a.spriteLayer < b.spriteLayer;
		return a.id < b.id;
	});
	for (const MapEditorSpriteQuad& q : quads)
		mapEditorEmitQuad(drawList, q);
	quads.clear();
}

inline void mapEditorAddSprite (ImDrawList* drawList, IFrontend* frontend, const SpritePtr& sprite,
		const ImVec2& pMin, const ImVec2& pMax, float alpha = 1.0f, int16_t angle = 0)
{
	if (!sprite || drawList == nullptr)
		return;
	for (Layer layer = LAYER_BACK; layer < MAX_LAYERS; ++layer) {
		const TexturePtr& texture = sprite->getActiveTexture(layer);
		if (!texture)
			continue;
		mapEditorAddTexture(drawList, frontend, texture.get(), pMin, pMax, alpha, angle);
	}
}
