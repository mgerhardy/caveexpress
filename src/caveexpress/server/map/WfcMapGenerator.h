#pragma once

#include "common/SpriteDefinition.h"
#include "common/ThemeType.h"
#include "common/EntityType.h"
#include "common/EmitterDefinition.h"
#include "common/IMap.h"
#include "caveexpress/shared/CaveTileDefinition.h"
#include <vector>
#include <cstdint>
#include <string>

namespace caveexpress {

/**
 * Wave Function Collapse generator for CaveExpress structure maps.
 *
 * Collapses an abstract terrain grid (Air / Solid / Ground) under adjacency
 * rules derived from playable CE maps, then instantiates theme sprites,
 * backgrounds, caves, player start and a few emitters.
 */
class WfcMapGenerator {
public:
	enum class Cell : uint8_t {
		Air = 0,
		Solid = 1,
		Ground = 2,
		Count = 3
	};

	struct Result {
		IMap::SettingsMap settings;
		IMap::StartPositions startPositions;
		std::vector<MapTileDefinition> tiles;
		std::vector<CaveTileDefinition> caves;
		std::vector<EmitterDefinition> emitters;
		std::string title;
		bool success = false;
	};

	WfcMapGenerator (const ThemeType& theme, unsigned int width, unsigned int height, unsigned int caveTarget = 3);

	Result generate (unsigned int seed);

private:
	const ThemeType* _theme;
	unsigned int _width;
	unsigned int _height;
	unsigned int _caveTarget;
	float _waterHeight = 1.5f;

	std::vector<SpriteDefPtr> _rocks;
	std::vector<SpriteDefPtr> _grounds;
	std::vector<SpriteDefPtr> _groundLeft;
	std::vector<SpriteDefPtr> _groundRight;
	std::vector<SpriteDefPtr> _backgrounds;
	std::vector<SpriteDefPtr> _caves;
	std::vector<SpriteDefPtr> _windows;

	void collectSprites ();
	bool collapse (std::vector<uint8_t>& domain, std::vector<Cell>& out, unsigned int& rng) const;
	bool propagate (std::vector<uint8_t>& domain, int x, int y) const;
	bool compatible (Cell a, Cell b, int dir) const;
	int entropy (uint8_t mask) const;
	Cell observe (uint8_t mask, unsigned int& rng) const;
	void applySeeds (std::vector<uint8_t>& domain) const;
	bool isAirConnected (const std::vector<Cell>& grid) const;
	void instantiate (const std::vector<Cell>& grid, Result& result, unsigned int& rng) const;
	SpriteDefPtr pick (const std::vector<SpriteDefPtr>& list, unsigned int& rng) const;
	Cell at (const std::vector<Cell>& grid, int x, int y, Cell fallback = Cell::Solid) const;
};

}
