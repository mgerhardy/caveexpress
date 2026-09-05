#pragma once

#include "imgui.h"
#include "common/IFrontend.h"
#include "common/Math.h"
#include "textures/Texture.h"
#include "textures/TextureCoords.h"
#include "sprites/Sprite.h"
#include "common/Layer.h"
#include <cmath>
#include <cstdint>

inline ImTextureID mapEditorToImTextureID (IFrontend* frontend, const Texture* texture)
{
	return static_cast<ImTextureID>(reinterpret_cast<uintptr_t>(frontend->getTextureData(texture)));
}

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

inline void mapEditorAddSprite (ImDrawList* drawList, IFrontend* frontend, const SpritePtr& sprite,
		const ImVec2& pMin, const ImVec2& pMax, float alpha = 1.0f, int16_t angle = 0)
{
	if (!sprite)
		return;
	for (Layer layer = LAYER_BACK; layer < MAX_LAYERS; ++layer) {
		const TexturePtr& texture = sprite->getActiveTexture(layer);
		if (!texture)
			continue;
		mapEditorAddTexture(drawList, frontend, texture.get(), pMin, pMax, alpha, angle);
	}
}
