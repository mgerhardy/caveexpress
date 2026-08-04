#pragma once

#include "common/SpriteDefinition.h"
#include "common/EntityType.h"
#include "common/Math.h"
#include <list>
#include <string>

enum MapEditorLayer {
	LAYER_NONE,
	LAYER_BACKGROUND,
	LAYER_SOLID,
	LAYER_FOREGROUND,
	LAYER_DECORATION,
	LAYER_EMITTER,
	LAYER_MAX
};

static const char* const MapEditorLayerNames[LAYER_MAX] = {
	"none", "background", "solid", "foreground", "decoration", "emitter"
};

struct MapEditorTileItem {
	SpriteDefPtr def;
	const EntityType* entityType = nullptr;
	int amount = 0;
	int delay = 0;
	gridCoord gridX = 0.0f;
	gridCoord gridY = 0.0f;
	MapEditorLayer layer = LAYER_NONE;
	EntityAngle angle = 0;
	std::string settings;
	bool mapTile = true;
	std::string linkId;
	float requiredWeight = 700.0f;
	float openAmount = 1.0f;

	bool operator== (const MapEditorTileItem& other) const;
	bool operator< (const MapEditorTileItem& other) const;

	gridCoord getX (bool useShape = false) const;
	gridCoord getY (bool useShape = false) const;
	vec2 getSize (bool useShape = false) const;
};

typedef std::list<MapEditorTileItem> MapEditorTileItems;

inline bool mapEditorLayerSort (const MapEditorTileItem& a, const MapEditorTileItem& b)
{
	return a.layer < b.layer;
}
