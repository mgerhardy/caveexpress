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

struct MapMetrics;

/**
 * Lua-tunable knobs for RandomMapGenerator.
 *
 * weights.air / weights.rock dominate collapse after border+platform seeds (most cells are
 * Air|Rock only). weightGround / ledge / undercut / shim mainly affect rare unseeded domains
 * and post-pass decoration; slopes are placed only in decorateSlopes, not observe.
 */
struct RandomMapRules {
	int caveTarget = 3;
	int minCaveSeparation = 3;
	float caveNpcChance = 0.75f;
	int caveNpcDelay = 5000;
	std::vector<std::string> caveNpcTypes; // entity names; empty → man/woman/grandpa

	bool windowsEnabled = true;
	bool oneWindowPerCave = true;
	bool forbidAdjacentWindows = true;

	int platformBandMin = 3;
	int platformBandMax = 4;
	int minVerticalGap = 3;
	int lengthMin = 4;
	int lengthMax = 7;
	int gapMin = 2;
	int gapMax = 4;
	float floatingChance = 0.28f;

	bool bridgesEnabled = true;
	int bridgeMaxGap = 3;

	int weightAir = 68;
	int weightRock = 32;
	int weightGround = 8;
	int weightLedge = 3;
	int weightUndercut = 4;
	int weightShim = 2;

	/** When true and the theme has package-target sprites, require one. Themes without targets
	 *  automatically fall back to NPC-transfer mode (see ThemeGameplayKit). */
	bool packageTargetRequired = true;
	bool packageTargetSidesWalkable = true;
	bool packageTargetRequireAirAbove = true;
	int minCavePackageAirSeparation = 4;
	int packageTransferCount = 3;

	int minPlatformLength = 3;
	int minSolidComponentSize = 4;
	int minPlatformRows = 3;
	int minWalkableCells = 18;
	int minColliderCells = 20;
	int minTreeEmitters = 1;

	int stoneChance = 4;
	int treeChance = 3;
	int walkingChance = 6;
	int packageChance = 2;
	int maxTrees = 3;

	int caveArtChance = 6;

	// Two acceptance stages: acceptsGrid then accepts. generate() also requires connected air.
	float minTotalScore = 40.0f;
	float maxExposedRockTopRatio = 0.55f;
	float maxOrphanColliderRatio = 0.20f;
	int maxGenerateAttempts = 96;
	int minSurfaceCells = 6;
	int minAirPercent = 25;
	bool requirePlayerStart = true;
	bool requireCaveIfAvailable = true;

	static RandomMapRules loadFromLua (const std::string& path = "random-map-rules.lua");
	bool acceptsGrid (int surfaceCells, int airCells, int mapCells) const;
	bool accepts (const MapMetrics& metrics, int mapWidth, int mapHeight, float waterHeight) const;
};

/** Per-theme gameplay mode resolved from assets + rules (not isIce hardcodes). */
struct ThemeGameplayKit {
	bool packageTargetRequired = false;
	const EntityType* packageEntity = nullptr;
	std::vector<const EntityType*> caveNpcs;
	int packageTransferCount = 0;
	int npcTransferCount = 0;
	int caveNpcDelay = 5000;
};

/**
 * Seeded structure generator for CaveExpress maps.
 *
 * Layout ownership is split:
 * - collapse: Air/Rock (+ ledge/undercut/shim/bridge bits) after forced platform-band seeds
 * - post-pass: cleanup, bridges, slopes, unreachable-air fill, walkable tops
 * - instantiate: sprite pick + caves/package/windows/emitters/starts
 *
 * When adding a Cell type, update compatible/observe and the relevant post-pass/instantiate path.
 */
class RandomMapGenerator {
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
		SlopeL = 9,
		SlopeR = 10,
		Count = 11
	};

	struct Result {
		IMap::SettingsMap settings;
		IMap::StartPositions startPositions;
		std::vector<MapTileDefinition> tiles;
		std::vector<CaveTileDefinition> caves;
		std::vector<EmitterDefinition> emitters;
		std::string title;
		std::string failureReason;
		bool success = false;
	};

	/**
	 * @param waterHeight World water rows used for borders/platforms/starts/settings.
	 *                    If <= 0, derived as clamp(height * 0.18, 1.0, 2.5).
	 * Cave count comes from rules.caveTarget only.
	 */
	RandomMapGenerator (const ThemeType& theme, unsigned int width, unsigned int height,
			const RandomMapRules& rules = RandomMapRules(), float waterHeight = 0.0f);

	Result generate (unsigned int seed);
	const RandomMapRules& rules () const { return _rules; }
	const ThemeGameplayKit& gameplayKit () const { return _kit; }

	struct SpriteBucketCounts {
		int rocksFull = 0;
		int grounds = 0;
		int groundsHanging = 0;
		int undercutL = 0;
		int undercutR = 0;
		int shims = 0;
		int slopesL = 0;
		int slopesR = 0;
		int bridges = 0;
		int caves = 0;
		int backgrounds = 0;
		int caveArt = 0;
		int packageTargets = 0;
	};
	SpriteBucketCounts spriteBucketCounts () const;

	// Exposed for unit tests of the constraint engine.
	bool propagateDomain (std::vector<uint16_t>& domain, int x, int y) const { return propagate(domain, x, y); }
	bool cellsCompatible (Cell a, Cell b, int dir) const { return compatible(a, b, dir); }
	Cell observeForTest (uint16_t mask, unsigned int& rng) const { return observe(mask, rng); }
	void clearUndercutShimBucketsForTest ()
	{
		_undercutL.clear();
		_undercutR.clear();
		_shims.clear();
	}
	static uint16_t cellBit (Cell c) { return static_cast<uint16_t>(1u << static_cast<unsigned>(c)); }

private:
	const ThemeType* _theme;
	unsigned int _width;
	unsigned int _height;
	float _waterHeight = 1.5f;
	RandomMapRules _rules;
	ThemeGameplayKit _kit;

	std::vector<SpriteDefPtr> _rocksFull;
	std::vector<SpriteDefPtr> _grounds;
	std::vector<SpriteDefPtr> _groundsHanging;
	std::vector<SpriteDefPtr> _groundLeft;
	std::vector<SpriteDefPtr> _groundRight;
	std::vector<SpriteDefPtr> _undercutL;
	std::vector<SpriteDefPtr> _undercutR;
	std::vector<SpriteDefPtr> _shims;
	std::vector<SpriteDefPtr> _slopesL;
	std::vector<SpriteDefPtr> _slopesR;
	std::vector<SpriteDefPtr> _backgrounds;
	std::vector<SpriteDefPtr> _caveArt;
	std::vector<SpriteDefPtr> _caves;
	std::vector<SpriteDefPtr> _windows;
	std::vector<SpriteDefPtr> _bridgeLeft;
	std::vector<SpriteDefPtr> _bridgeRight;
	std::vector<SpriteDefPtr> _bridgePlank;
	std::vector<SpriteDefPtr> _packageTargets;

	void collectSprites ();
	void resolveGameplayKit ();
	static const EntityType* resolvePackageEntity (const ThemeType& theme);
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
	void decorateSlopes (std::vector<Cell>& grid) const;
	void ensureWalkableTops (std::vector<Cell>& grid) const;
	void placeBridgeSpans (std::vector<Cell>& grid, unsigned int& rng) const;
	void cleanupClutter (std::vector<Cell>& grid) const;
	void removeSmallSolidComponents (std::vector<Cell>& grid) const;
	void removeShortPlatforms (std::vector<Cell>& grid) const;
	void fillEnclosedAirPockets (std::vector<Cell>& grid) const;
	void relabelPlatformEdges (std::vector<Cell>& grid) const;
	void enforceWaterClearance (std::vector<Cell>& grid) const;
	float waterSurfaceY () const;
	int maxWalkableRow () const;
	int waterBedStartRow () const;
	int findOpenAirSeed (const std::vector<Cell>& grid) const;
	void floodAir (const std::vector<Cell>& grid, int startIdx, std::vector<uint8_t>& seen, int& reached) const;
	void fillUnreachableAir (std::vector<Cell>& grid) const;
	bool isAirConnected (const std::vector<Cell>& grid) const;
	int airPathDistance (const std::vector<Cell>& grid, int sx, int sy, int gx, int gy) const;
	bool isWalkableSurface (Cell c) const;
	bool isColliderSolid (Cell c) const;
	void instantiate (const std::vector<Cell>& grid, Result& result, unsigned int& rng) const;
	SpriteDefPtr pick (const std::vector<SpriteDefPtr>& list, unsigned int& rng) const;
	Cell at (const std::vector<Cell>& grid, int x, int y, Cell fallback = Cell::Rock) const;
	void setTile (Result& result, int x, int y, const SpriteDefPtr& def) const;
};

}
