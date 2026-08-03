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

struct WfcRules {
	int caveTarget = 3;
	int minCaveSeparation = 3;
	float caveNpcChance = 0.75f;

	bool windowsEnabled = true;
	bool oneWindowPerCave = true;
	bool forbidAdjacentWindows = true;

	int platformBandMin = 2;
	int platformBandMax = 3;
	int minVerticalGap = 5;
	int lengthMin = 3;
	int lengthMax = 5;
	int gapMin = 2;
	int gapMax = 4;
	float floatingChance = 0.2f;

	bool bridgesEnabled = true;
	int bridgeMaxGap = 3;

	int weightAir = 60;
	int weightRock = 40;
	int weightGround = 8;
	int weightLedge = 3;
	int weightUndercut = 4;
	int weightShim = 2;

	bool packageTargetRequired = true;
	bool packageTargetSidesWalkable = true;
	bool packageTargetRequireAirAbove = true;

	int stoneChance = 5;
	int treeChance = 6;
	int walkingChance = 8;
	int packageChance = 2;

	int caveArtChance = 6;

	static WfcRules loadFromLua (const std::string& path = "wfc-rules.lua");
};

/**
 * Structure-aware Wave Function Collapse for CaveExpress maps.
 */
class WfcMapGenerator {
public:
	enum class Cell : uint8_t {
		Air = 0,
		Rock = 1,
		Ground = 2,
		LedgeL = 3,
		LedgeR = 4,
		UndercutL = 5,
		UndercutR = 6,
		Shim = 7,
		Bridge = 8,
		Count = 9
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

	WfcMapGenerator (const ThemeType& theme, unsigned int width, unsigned int height,
			unsigned int caveTarget = 3, const WfcRules& rules = WfcRules());

	Result generate (unsigned int seed);
	const WfcRules& rules () const { return _rules; }

private:
	const ThemeType* _theme;
	unsigned int _width;
	unsigned int _height;
	unsigned int _caveTarget;
	float _waterHeight = 1.5f;
	WfcRules _rules;

	std::vector<SpriteDefPtr> _rocksFull;
	std::vector<SpriteDefPtr> _grounds;
	std::vector<SpriteDefPtr> _groundsHanging; // thin platforms (e.g. tile-ground-06) that need air below
	std::vector<SpriteDefPtr> _groundLeft;
	std::vector<SpriteDefPtr> _groundRight;
	std::vector<SpriteDefPtr> _undercutL;
	std::vector<SpriteDefPtr> _undercutR;
	std::vector<SpriteDefPtr> _shims;
	std::vector<SpriteDefPtr> _backgrounds;
	std::vector<SpriteDefPtr> _caveArt;
	std::vector<SpriteDefPtr> _caves;
	std::vector<SpriteDefPtr> _windows;
	std::vector<SpriteDefPtr> _bridgeLeft;
	std::vector<SpriteDefPtr> _bridgeRight;
	std::vector<SpriteDefPtr> _bridgePlank;
	std::vector<SpriteDefPtr> _packageTargets;

	void collectSprites ();
	bool isFullRockSprite (const SpriteDefPtr& def) const;
	bool isUndercutLeftSprite (const SpriteDefPtr& def) const;
	bool isUndercutRightSprite (const SpriteDefPtr& def) const;
	bool isShimSprite (const SpriteDefPtr& def) const;
	bool isHangingGroundSprite (const SpriteDefPtr& def) const;

	bool collapse (std::vector<uint16_t>& domain, std::vector<Cell>& out, unsigned int& rng) const;
	bool propagate (std::vector<uint16_t>& domain, int x, int y) const;
	bool compatible (Cell a, Cell b, int dir) const;
	int entropy (uint16_t mask) const;
	Cell observe (uint16_t mask, unsigned int& rng) const;
	void applyBorderSeeds (std::vector<uint16_t>& domain) const;
	void seedPlatforms (std::vector<uint16_t>& domain, unsigned int& rng) const;
	void decorateEdges (std::vector<Cell>& grid, unsigned int& rng) const;
	void ensureWalkableTops (std::vector<Cell>& grid) const;
	void placeBridgeSpans (std::vector<Cell>& grid, unsigned int& rng) const;
	bool isAirConnected (const std::vector<Cell>& grid) const;
	bool isWalkableSurface (Cell c) const;
	bool isColliderSolid (Cell c) const;
	void instantiate (const std::vector<Cell>& grid, Result& result, unsigned int& rng) const;
	SpriteDefPtr pick (const std::vector<SpriteDefPtr>& list, unsigned int& rng) const;
	Cell at (const std::vector<Cell>& grid, int x, int y, Cell fallback = Cell::Rock) const;
	void setTile (Result& result, int x, int y, const SpriteDefPtr& def) const;
};

}
