#pragma once

#include "common/IMap.h"
#include "common/MapTileDefinition.h"
#include "common/EmitterDefinition.h"
#include "caveexpress/shared/CaveTileDefinition.h"
#include <cstdint>
#include <string>
#include <vector>

namespace caveexpress {

struct MapMetrics {
	bool valid = false;

	int width = 0;
	int height = 0;
	int flyableCells = 0;
	int solidCells = 0;
	int walkableCells = 0;
	int caveCount = 0;
	int packageTargetCount = 0;
	int packageEmitterCount = 0;
	int windowCount = 0;

	int cavesReachable = 0;
	int packageTargetsReachable = 0;
	int packagesReachableToTarget = 0;

	int flyableReachable = 0;
	int unreachableFlyable = 0;
	float flyableReachabilityRatio = 0.0f;

	int exposedRockTops = 0;
	int solidWithAirAbove = 0;
	int orphanColliders = 0;
	int colliderCells = 0;
	int windowWindowAdjacencies = 0;
	int cavesTooClose = 0;
	int packageTargetsWithBadNiche = 0;
	int bridgesWithoutBackground = 0;
	int cavesAbovePackageTarget = 0;
	int cavePackageAirTooClose = 0;
	/** Cave cells painted as colliders (e.g. covered by a larger rock tile). Bridge overlays do not count. */
	int cavesCoveredBySolid = 0;
	/** Cave cells that still have a host tile (background, rock, window, ...) occupying them. Overlays (bridge, liane) do not count. */
	int cavesOverlappingTiles = 0;
	/** Caves with no NPC-walkable ground, ledge, or bridge in the cell immediately below. */
	int cavesMissingPlatform = 0;
	int shortPlatformRuns = 0;
	int smallSolidComponents = 0;
	int isolatedWalkables = 0;
	int platformRows = 0;
	int treeEmitterCount = 0;

	float walkableSurfaceRatio = 0.0f;
	float exposedRockTopRatio = 0.0f;
	float orphanColliderRatio = 0.0f;
	float airOpenness = 0.0f;
	float meanPlatformLength = 0.0f;
	float minCaveSeparation = 0.0f;
	float minCavePackageAirPath = 0.0f;

	float totalScore = 0.0f;
	std::string failureReason;
};

/**
 * Static check that a map's settings and placed objects can satisfy the win
 * conditions. Catches Lua mistakes such as packagetransfercount with no
 * shredder, npctransfercount with fewer than two caves, or a 1x2 waterfall
 * covering another solid. Cutscenes skip package/NPC quota checks but still
 * validate cave placement, empty cells, and occupying-tile overlaps.
 */
struct MapWinCondition {
	bool winnable = true;
	std::vector<std::string> issues;
};

/**
 * Static quality analysis for CaveExpress maps (hand-authored or random).
 * Reachability uses flyable-cell flood fill from the player start (flying game).
 * Gates are treated as flyable (a pressure plate can open them).
 * Full flyable coverage is reported in metrics; random generation enforces it as a hard gate.
 */
class MapValidator {
public:
	MapMetrics evaluate (int width, int height,
			const std::vector<MapTileDefinition>& tiles,
			const std::vector<CaveTileDefinition>& caves,
			const std::vector<EmitterDefinition>& emitters,
			const IMap::StartPositions& starts,
			int minCaveSeparation = 3,
			int minCavePackageAirSeparation = 4,
			int minPlatformLength = 3,
			int minSolidComponentSize = 4) const;

	static MapWinCondition checkWinConditions (const IMap::SettingsMap& settings,
			const std::vector<MapTileDefinition>& tiles,
			const std::vector<CaveTileDefinition>& caves,
			const std::vector<EmitterDefinition>& emitters);

private:
	enum class CellKind : uint8_t {
		Empty = 0,
		FlyableDecor, // background / window / cave art (flyable)
		Walkable,     // ground / bridge
		Collider      // rock and other solids
	};

	struct Grid {
		int width = 0;
		int height = 0;
		std::vector<CellKind> kind;
		std::vector<uint8_t> isWindow;
		std::vector<uint8_t> isCave;
		std::vector<uint8_t> isPackageTarget;
		std::vector<uint8_t> isBridge;
		std::vector<uint8_t> isBackground;

		int idx (int x, int y) const { return x + y * width; }
		bool inBounds (int x, int y) const {
			return x >= 0 && y >= 0 && x < width && y < height;
		}
		bool flyable (int x, int y) const {
			if (!inBounds(x, y))
				return false;
			const CellKind k = kind[idx(x, y)];
			return k == CellKind::Empty || k == CellKind::FlyableDecor;
		}
		bool solid (int x, int y) const {
			if (!inBounds(x, y))
				return true; // out of bounds blocks
			const CellKind k = kind[idx(x, y)];
			return k == CellKind::Walkable || k == CellKind::Collider;
		}
	};

	void paintSprite (Grid& grid, const SpriteDefPtr& def, int x, int y, CellKind kind) const;
	CellKind classifyTile (const SpriteType& type) const;
	void floodFlyable (const Grid& grid, int sx, int sy, std::vector<uint8_t>& reached) const;
	int airPathDistance (const Grid& grid, int sx, int sy, int gx, int gy) const;
};

}
