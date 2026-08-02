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
 * Structure-aware Wave Function Collapse for CaveExpress maps.
 *
 * Abstract cells encode platform grammar taken from hand-authored maps and
 * sprite physics shapes in sprites.lua:
 *   - Ground / ledge surfaces over rock fill
 *   - Undercut slopes (slope-*-02) and shims at hanging edges
 *   - Background air with caves/windows on platform façades
 *
 * Platform rows are seeded (length 3–5, gaps 2–4, vertical spacing ~3–5),
 * then remaining cells collapse under adjacency sockets derived from those shapes.
 */
class WfcMapGenerator {
public:
	enum class Cell : uint8_t {
		Air = 0,
		Rock = 1,
		Ground = 2,
		LedgeL = 3,
		LedgeR = 4,
		UndercutL = 5, // tile-rock-slope-*-left-02 (solid SW)
		UndercutR = 6, // tile-rock-slope-*-right-02 (solid SE)
		Shim = 7,      // hanging tip under rock
		Count = 8
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

	std::vector<SpriteDefPtr> _rocksFull;
	std::vector<SpriteDefPtr> _grounds;
	std::vector<SpriteDefPtr> _groundLeft;
	std::vector<SpriteDefPtr> _groundRight;
	std::vector<SpriteDefPtr> _undercutL;
	std::vector<SpriteDefPtr> _undercutR;
	std::vector<SpriteDefPtr> _shims;
	std::vector<SpriteDefPtr> _backgrounds;
	std::vector<SpriteDefPtr> _caveArt;
	std::vector<SpriteDefPtr> _caves;
	std::vector<SpriteDefPtr> _windows;

	void collectSprites ();
	bool isFullRockSprite (const SpriteDefPtr& def) const;
	bool isUndercutLeftSprite (const SpriteDefPtr& def) const;
	bool isUndercutRightSprite (const SpriteDefPtr& def) const;
	bool isShimSprite (const SpriteDefPtr& def) const;

	bool collapse (std::vector<uint16_t>& domain, std::vector<Cell>& out, unsigned int& rng) const;
	bool propagate (std::vector<uint16_t>& domain, int x, int y) const;
	bool compatible (Cell a, Cell b, int dir) const;
	int entropy (uint16_t mask) const;
	Cell observe (uint16_t mask, unsigned int& rng) const;
	void applyBorderSeeds (std::vector<uint16_t>& domain) const;
	void seedPlatforms (std::vector<uint16_t>& domain, unsigned int& rng) const;
	void decorateEdges (std::vector<Cell>& grid, unsigned int& rng) const;
	bool isAirConnected (const std::vector<Cell>& grid) const;
	bool isSurface (Cell c) const;
	void instantiate (const std::vector<Cell>& grid, Result& result, unsigned int& rng) const;
	SpriteDefPtr pick (const std::vector<SpriteDefPtr>& list, unsigned int& rng) const;
	Cell at (const std::vector<Cell>& grid, int x, int y, Cell fallback = Cell::Rock) const;
	void setTile (Result& result, int x, int y, const SpriteDefPtr& def) const;
};

}
