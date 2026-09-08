#include "tests/TestShared.h"
#include "caveexpress/shared/MapValidator.h"
#include "caveexpress/shared/CaveExpressMapContext.h"
#include "caveexpress/shared/CaveExpressEntityType.h"
#include "caveexpress/server/map/RandomMapGenerator.h"
#include "common/ThemeType.h"
#include "common/TextureDefinition.h"
#include "common/SpriteDefinition.h"
#include "common/MapSettings.h"
#include "common/MapManager.h"
#include "common/Log.h"
#include "common/String.h"
#include <sstream>
#include <vector>

namespace caveexpress {

class MapValidatorTest: public AbstractTest {
protected:
	TextureDefinition* _textures = nullptr;

	void SetUp () override
	{
		AbstractTest::SetUp();
		_textures = new TextureDefinition("small");
		SpriteDefinition::get().init(*_textures);
	}

	void TearDown () override
	{
		delete _textures;
		_textures = nullptr;
		AbstractTest::TearDown();
	}

	MapMetrics evaluateContext (CaveExpressMapContext& ctx) const
	{
		const int w = string::toInt(ctx.getSettings().at(msn::WIDTH));
		const int h = string::toInt(ctx.getSettings().at(msn::HEIGHT));
		std::vector<MapTileDefinition> tiles = ctx.getMapTileDefinitions();
		for (const GateDefinition& gate : ctx.getGateDefinitions())
			tiles.emplace_back(gate.x, gate.y, gate.spriteDef, 0);
		for (const PressurePlateDefinition& plate : ctx.getPressurePlateDefinitions())
			tiles.emplace_back(plate.x, plate.y, plate.spriteDef, 0);
		return MapValidator().evaluate(w, h, tiles, ctx.getCaveTileDefinitions(),
				ctx.getEmitterDefinitions(), ctx.getStartPositions());
	}

	SpriteDefPtr requireSprite (const char* id) const
	{
		const SpriteDefPtr def = SpriteDefinition::get().getSpriteDefinition(id);
		EXPECT_TRUE(!!def) << id;
		return def;
	}

	void addTile (std::vector<MapTileDefinition>& tiles, const char* id, int x, int y, EntityAngle angle = 0) const
	{
		const SpriteDefPtr def = requireSprite(id);
		if (def)
			tiles.emplace_back(static_cast<gridCoord>(x), static_cast<gridCoord>(y), def, angle);
	}

	/** Open flyable map with rock border; start at (1,1). */
	void fillOpenBorder (std::vector<MapTileDefinition>& tiles, int w, int h) const
	{
		for (int y = 0; y < h; ++y) {
			for (int x = 0; x < w; ++x) {
				if (x == 0 || y == 0 || x == w - 1 || y == h - 1)
					addTile(tiles, "tile-rock-01", x, y);
				else
					addTile(tiles, "tile-background-01", x, y);
			}
		}
	}
};

TEST_F(MapValidatorTest, testAllMapsValidate)
{
	LUAMapManager mgr;
	mgr.loadMaps();
	ASSERT_FALSE(mgr.getMaps().empty());

	std::ostringstream failed;
	int checked = 0;
	for (const auto& entry : mgr.getMaps()) {
		const std::string& id = entry.first;
		if (string::startsWith(id, "test") || string::startsWith(id, "empty"))
			continue;
		CaveExpressMapContext ctx(id);
		if (!ctx.load(true)) {
			failed << id << ": failed to load\n";
			continue;
		}
		++checked;
		const MapMetrics m = evaluateContext(ctx);
		if (m.valid)
			continue;
		failed << id << ": " << m.failureReason
				<< " (caves " << m.cavesReachable << "/" << m.caveCount
				<< ", covered=" << m.cavesCoveredBySolid
				<< ", overlap=" << m.cavesOverlappingTiles
				<< ", noPlatform=" << m.cavesMissingPlatform
				<< ", targets " << m.packageTargetsReachable << "/" << m.packageTargetCount
				<< ")\n";
	}
	EXPECT_GT(checked, 0);
	EXPECT_TRUE(failed.str().empty()) << "maps that fail MapValidator::evaluate:\n" << failed.str();
}

TEST_F(MapValidatorTest, testHandMapBaseline)
{
	LUAMapManager mgr;
	mgr.loadMaps();
	ASSERT_FALSE(mgr.getMaps().empty());

	int checked = 0;
	for (const auto& entry : mgr.getMaps()) {
		const std::string& name = entry.first;
		if (string::startsWith(name, "test") || string::startsWith(name, "empty"))
			continue;
		CaveExpressMapContext ctx(name);
		ASSERT_TRUE(ctx.load(true)) << name;
		const MapMetrics m = evaluateContext(ctx);
		Log::info(LOG_GAMEIMPL,
				"hand map %s score=%.1f valid=%i exposed=%.3f orphan=%.3f caves=%i/%i",
				name.c_str(), m.totalScore, m.valid ? 1 : 0, m.exposedRockTopRatio, m.orphanColliderRatio,
				m.cavesReachable, m.caveCount);
		++checked;
		EXPECT_TRUE(m.valid) << name << ": " << m.failureReason;
	}
	EXPECT_GT(checked, 0);
}

TEST_F(MapValidatorTest, testRandomMapAcceptedMapsMeetRules)
{
	RandomMapRules rules = RandomMapRules::loadFromLua();
	rules.caveTarget = 2;
	const unsigned int seeds[] = { 42u, 7u, 4242u, 2017u, 3030u };
	for (unsigned int seed : seeds) {
		const ThemeType& theme = (seed % 2u == 0) ? ThemeTypes::ROCK : ThemeTypes::ICE;
		RandomMapGenerator gen(theme, 18, 12, rules);
		const RandomMapGenerator::Result result = gen.generate(seed);
		ASSERT_TRUE(result.success) << "seed " << seed;
		const int w = string::toInt(result.settings.at(msn::WIDTH));
		const int h = string::toInt(result.settings.at(msn::HEIGHT));
		const MapMetrics m = MapValidator().evaluate(w, h, result.tiles, result.caves, result.emitters,
				result.startPositions, rules.minCaveSeparation, rules.minCavePackageAirSeparation,
				rules.minPlatformLength, rules.minSolidComponentSize);
		EXPECT_TRUE(rules.accepts(m, w, h, string::toFloat(result.settings.at(msn::WATER_HEIGHT))))
				<< "seed " << seed << ": " << m.failureReason;
		EXPECT_EQ(0, m.cavesAbovePackageTarget) << "seed " << seed;
		EXPECT_EQ(0, m.shortPlatformRuns) << "seed " << seed;
		EXPECT_EQ(0, m.windowWindowAdjacencies) << "seed " << seed;
	}
}

TEST_F(MapValidatorTest, testValidatorCatchesUnreachableAir)
{
	std::vector<MapTileDefinition> tiles;
	std::vector<CaveTileDefinition> caves;
	std::vector<EmitterDefinition> emitters;
	IMap::StartPositions starts;
	starts.push_back({ "1", "1" });

	const int w = 6;
	const int h = 6;
	for (int y = 0; y < h; ++y) {
		for (int x = 0; x < w; ++x) {
			const bool outer = x == 0 || y == 0 || x == w - 1 || y == h - 1;
			const bool pocketWall = (x >= 2 && x <= 4 && y >= 2 && y <= 4 && !(x == 3 && y == 3));
			if (outer || pocketWall)
				addTile(tiles, "tile-rock-01", x, y);
			else
				addTile(tiles, "tile-background-01", x, y);
		}
	}

	const MapMetrics m = MapValidator().evaluate(w, h, tiles, caves, emitters, starts);
	EXPECT_GT(m.unreachableFlyable, 0) << m.failureReason;
}

TEST_F(MapValidatorTest, testMetricCaveAbovePackageTarget)
{
	std::vector<MapTileDefinition> tiles;
	std::vector<CaveTileDefinition> caves;
	std::vector<EmitterDefinition> emitters;
	IMap::StartPositions starts = { { "1", "1" } };
	const int w = 8;
	const int h = 8;
	fillOpenBorder(tiles, w, h);

	// Same column: cave above package target.
	addTile(tiles, "tile-cave-01", 3, 2);
	addTile(tiles, "tile-ground-01", 2, 5);
	addTile(tiles, "tile-packagetarget-rock-01-idle", 3, 5);
	addTile(tiles, "tile-ground-01", 4, 5);
	addTile(tiles, "tile-rock-01", 3, 6);

	const MapMetrics m = MapValidator().evaluate(w, h, tiles, caves, emitters, starts);
	EXPECT_GT(m.cavesAbovePackageTarget, 0);
}

TEST_F(MapValidatorTest, testMetricShortPlatformRun)
{
	std::vector<MapTileDefinition> tiles;
	std::vector<CaveTileDefinition> caves;
	std::vector<EmitterDefinition> emitters;
	IMap::StartPositions starts = { { "1", "1" } };
	const int w = 8;
	const int h = 6;
	fillOpenBorder(tiles, w, h);
	addTile(tiles, "tile-ground-01", 3, 3); // length-1 run

	const MapMetrics m = MapValidator().evaluate(w, h, tiles, caves, emitters, starts,
			3, 4, /*minPlatformLength=*/3, 4);
	EXPECT_GT(m.shortPlatformRuns, 0);
	EXPECT_GE(m.isolatedWalkables, 1);
}

TEST_F(MapValidatorTest, testRotatedPackageTargetUsesIntakeSide)
{
	std::vector<MapTileDefinition> tiles;
	std::vector<CaveTileDefinition> caves;
	std::vector<EmitterDefinition> emitters;
	IMap::StartPositions starts = { { "1", "1" } };
	const int w = 8;
	const int h = 8;
	fillOpenBorder(tiles, w, h);
	// Seal every neighbour of (3,3) except the intake cell below.
	addTile(tiles, "tile-rock-01", 3, 2);
	addTile(tiles, "tile-rock-01", 2, 3);
	addTile(tiles, "tile-rock-01", 4, 3);
	addTile(tiles, "tile-packagetarget-rock-01-idle", 3, 3, 180);

	const MapMetrics rotated = MapValidator().evaluate(w, h, tiles, caves, emitters, starts);
	EXPECT_EQ(1, rotated.packageTargetsReachable) << rotated.failureReason;
	EXPECT_TRUE(rotated.valid) << rotated.failureReason;

	tiles.clear();
	fillOpenBorder(tiles, w, h);
	addTile(tiles, "tile-rock-01", 3, 2);
	addTile(tiles, "tile-rock-01", 2, 3);
	addTile(tiles, "tile-rock-01", 4, 3);
	addTile(tiles, "tile-packagetarget-rock-01-idle", 3, 3, 0);
	const MapMetrics upright = MapValidator().evaluate(w, h, tiles, caves, emitters, starts);
	// Upright intake is above, which is rock — still reachable via the flyable neighbour below.
	EXPECT_EQ(1, upright.packageTargetsReachable) << upright.failureReason;
}

TEST_F(MapValidatorTest, testGeyserOnPipeCountsAsPackageDelivery)
{
	std::vector<MapTileDefinition> tiles;
	std::vector<CaveTileDefinition> caves;
	std::vector<EmitterDefinition> emitters;
	IMap::StartPositions starts = { { "1", "1" } };
	const int w = 8;
	const int h = 8;
	fillOpenBorder(tiles, w, h);
	addTile(tiles, "tile-rock-01", 3, 4);
	addTile(tiles, "tile-rock-01", 2, 5);
	addTile(tiles, "tile-rock-01", 3, 6);
	addTile(tiles, "tile-packagetarget-ice-01-idle", 3, 5);
	addTile(tiles, "tile-geyser-ice-01-active", 4, 5);

	const MapMetrics m = MapValidator().evaluate(w, h, tiles, caves, emitters, starts);
	EXPECT_EQ(1, m.packageTargetsReachable) << m.failureReason;
}

TEST_F(MapValidatorTest, testMetricPackageTargetBadNiche)
{
	std::vector<MapTileDefinition> tiles;
	std::vector<CaveTileDefinition> caves;
	std::vector<EmitterDefinition> emitters;
	IMap::StartPositions starts = { { "1", "1" } };
	const int w = 8;
	const int h = 6;
	fillOpenBorder(tiles, w, h);
	// Target with air on both sides — missing walkable L/R niche.
	addTile(tiles, "tile-packagetarget-rock-01-idle", 3, 3);
	addTile(tiles, "tile-rock-01", 3, 4);

	const MapMetrics goodMissing = MapValidator().evaluate(w, h, tiles, caves, emitters, starts);
	EXPECT_GT(goodMissing.packageTargetsWithBadNiche, 0);

	tiles.clear();
	fillOpenBorder(tiles, w, h);
	addTile(tiles, "tile-ground-01", 2, 3);
	addTile(tiles, "tile-packagetarget-rock-01-idle", 3, 3);
	addTile(tiles, "tile-ground-01", 4, 3);
	addTile(tiles, "tile-rock-01", 3, 4);
	const MapMetrics nicheOk = MapValidator().evaluate(w, h, tiles, caves, emitters, starts);
	EXPECT_EQ(0, nicheOk.packageTargetsWithBadNiche);
}

TEST_F(MapValidatorTest, testMetricWindowWindowAdjacency)
{
	std::vector<MapTileDefinition> tiles;
	std::vector<CaveTileDefinition> caves;
	std::vector<EmitterDefinition> emitters;
	IMap::StartPositions starts = { { "1", "1" } };
	const int w = 8;
	const int h = 6;
	fillOpenBorder(tiles, w, h);
	addTile(tiles, "tile-background-window-01", 2, 2);
	addTile(tiles, "tile-background-window-02", 3, 2);

	const MapMetrics m = MapValidator().evaluate(w, h, tiles, caves, emitters, starts);
	EXPECT_GT(m.windowWindowAdjacencies, 0);

	tiles.clear();
	fillOpenBorder(tiles, w, h);
	addTile(tiles, "tile-background-window-01", 2, 2);
	addTile(tiles, "tile-background-window-02", 4, 2);
	const MapMetrics spaced = MapValidator().evaluate(w, h, tiles, caves, emitters, starts);
	EXPECT_EQ(0, spaced.windowWindowAdjacencies);
}

TEST_F(MapValidatorTest, testAcceptsRejectsIndividualGates)
{
	RandomMapRules rules;
	rules.minPlatformRows = 0;
	rules.minWalkableCells = 0;
	rules.minColliderCells = 0;
	rules.minTreeEmitters = 0;
	rules.minTotalScore = -999.0f;
	rules.maxExposedRockTopRatio = 1.0f;
	rules.maxOrphanColliderRatio = 1.0f;

	MapMetrics m;
	m.valid = true;
	EXPECT_TRUE(rules.accepts(m, 10, 10, 1.0f));

	m.cavesAbovePackageTarget = 1;
	EXPECT_FALSE(rules.accepts(m, 10, 10, 1.0f));
	m.cavesAbovePackageTarget = 0;

	m.shortPlatformRuns = 1;
	EXPECT_FALSE(rules.accepts(m, 10, 10, 1.0f));
	m.shortPlatformRuns = 0;

	m.windowWindowAdjacencies = 1;
	EXPECT_FALSE(rules.accepts(m, 10, 10, 1.0f));
	m.windowWindowAdjacencies = 0;

	m.cavePackageAirTooClose = 1;
	EXPECT_FALSE(rules.accepts(m, 10, 10, 1.0f));
	m.cavePackageAirTooClose = 0;

	m.packageTargetsWithBadNiche = 1;
	EXPECT_FALSE(rules.accepts(m, 10, 10, 1.0f));
	m.packageTargetsWithBadNiche = 0;

	m.bridgesWithoutBackground = 1;
	EXPECT_FALSE(rules.accepts(m, 10, 10, 1.0f));
}

TEST_F(MapValidatorTest, testCaveCoveredByMultiCellSolid)
{
	std::vector<MapTileDefinition> tiles;
	std::vector<CaveTileDefinition> caves;
	std::vector<EmitterDefinition> emitters;
	IMap::StartPositions starts = { { "3", "2" } };
	const int w = 8;
	const int h = 8;
	fillOpenBorder(tiles, w, h);
	for (int x = 1; x < w - 1; ++x)
		addTile(tiles, "tile-ground-04", x, 5);

	// 2x2 rock at (0,3) covers (0,3)(1,3)(0,4)(1,4) — same bug as the intro movie map.
	addTile(tiles, "tile-rock-big-01", 0, 3);
	const SpriteDefPtr caveDef = requireSprite("tile-cave-01");
	ASSERT_TRUE(!!caveDef);
	caves.emplace_back(1, 4, caveDef, EntityTypes::NPC_FRIENDLY_MAN, 5000);

	const MapMetrics m = MapValidator().evaluate(w, h, tiles, caves, emitters, starts);
	EXPECT_GT(m.cavesCoveredBySolid, 0);
	EXPECT_FALSE(m.valid);
	EXPECT_EQ("cave covered by solid", m.failureReason);
}

TEST_F(MapValidatorTest, testCaveOverlapsBackgroundTile)
{
	std::vector<MapTileDefinition> tiles;
	std::vector<CaveTileDefinition> caves;
	std::vector<EmitterDefinition> emitters;
	IMap::StartPositions starts = { { "3", "2" } };
	const int w = 8;
	const int h = 8;
	fillOpenBorder(tiles, w, h);
	for (int x = 1; x < w - 1; ++x)
		addTile(tiles, "tile-ground-04", x, 5);
	addTile(tiles, "tile-background-01", 2, 4);
	const SpriteDefPtr caveDef = requireSprite("tile-cave-01");
	ASSERT_TRUE(!!caveDef);
	caves.emplace_back(2, 4, caveDef, EntityType::NONE, 1000);

	const MapMetrics m = MapValidator().evaluate(w, h, tiles, caves, emitters, starts);
	EXPECT_GT(m.cavesOverlappingTiles, 0);
	EXPECT_FALSE(m.valid);
	EXPECT_EQ("cave overlaps another tile", m.failureReason);
}

TEST_F(MapValidatorTest, testCaveAllowsBridgeOverlay)
{
	std::vector<MapTileDefinition> tiles;
	std::vector<CaveTileDefinition> caves;
	std::vector<EmitterDefinition> emitters;
	IMap::StartPositions starts = { { "3", "2" } };
	const int w = 8;
	const int h = 8;
	for (int y = 0; y < h; ++y) {
		for (int x = 0; x < w; ++x) {
			if (x == 0 || y == 0 || x == w - 1 || y == h - 1)
				addTile(tiles, "tile-rock-01", x, y);
			else if (!(x == 2 && y == 4))
				addTile(tiles, "tile-background-01", x, y);
		}
	}
	for (int x = 1; x < w - 1; ++x)
		addTile(tiles, "tile-ground-04", x, 5);
	addTile(tiles, "bridge-plank-01", 2, 4);
	const SpriteDefPtr caveDef = requireSprite("tile-cave-01");
	ASSERT_TRUE(!!caveDef);
	caves.emplace_back(2, 4, caveDef, EntityType::NONE, 1000);

	const MapMetrics m = MapValidator().evaluate(w, h, tiles, caves, emitters, starts);
	EXPECT_EQ(0, m.cavesOverlappingTiles);
	EXPECT_EQ(0, m.cavesCoveredBySolid);
	EXPECT_TRUE(m.valid) << m.failureReason;
}

TEST_F(MapValidatorTest, testCaveMissingPlatformBelow)
{
	std::vector<MapTileDefinition> tiles;
	std::vector<CaveTileDefinition> caves;
	std::vector<EmitterDefinition> emitters;
	IMap::StartPositions starts = { { "3", "2" } };
	const int w = 8;
	const int h = 8;
	for (int y = 0; y < h; ++y) {
		for (int x = 0; x < w; ++x) {
			if (x == 0 || y == 0 || x == w - 1 || y == h - 1)
				addTile(tiles, "tile-rock-01", x, y);
		}
	}
	const SpriteDefPtr caveDef = requireSprite("tile-cave-01");
	ASSERT_TRUE(!!caveDef);
	caves.emplace_back(2, 4, caveDef, EntityType::NONE, 1000);

	const MapMetrics m = MapValidator().evaluate(w, h, tiles, caves, emitters, starts);
	EXPECT_EQ(0, m.cavesOverlappingTiles);
	EXPECT_GT(m.cavesMissingPlatform, 0);
	EXPECT_FALSE(m.valid);
	EXPECT_EQ("cave has no ground, ledge, or bridge below", m.failureReason);
}

TEST_F(MapValidatorTest, testCavePlatformAcceptsLedgeAndBridgeRejectsRock)
{
	std::vector<MapTileDefinition> tiles;
	std::vector<CaveTileDefinition> caves;
	std::vector<EmitterDefinition> emitters;
	IMap::StartPositions starts = { { "3", "2" } };
	const int w = 8;
	const int h = 8;
	const SpriteDefPtr caveDef = requireSprite("tile-cave-01");
	ASSERT_TRUE(!!caveDef);
	caves.emplace_back(2, 4, caveDef, EntityType::NONE, 1000);

	auto paintHost = [&] (const char* belowId) {
		tiles.clear();
		for (int y = 0; y < h; ++y) {
			for (int x = 0; x < w; ++x) {
				if (x == 0 || y == 0 || x == w - 1 || y == h - 1)
					addTile(tiles, "tile-rock-01", x, y);
				else if (!(x == 2 && y == 4))
					addTile(tiles, "tile-background-01", x, y);
			}
		}
		addTile(tiles, belowId, 2, 5);
	};

	paintHost("tile-ground-ledge-desert-left-01");
	MapMetrics ledge = MapValidator().evaluate(w, h, tiles, caves, emitters, starts);
	EXPECT_EQ(0, ledge.cavesMissingPlatform);
	EXPECT_TRUE(ledge.valid) << ledge.failureReason;

	paintHost("bridge-plank-01");
	MapMetrics bridge = MapValidator().evaluate(w, h, tiles, caves, emitters, starts);
	EXPECT_EQ(0, bridge.cavesMissingPlatform);
	EXPECT_TRUE(bridge.valid) << bridge.failureReason;

	paintHost("tile-rock-01");
	MapMetrics rock = MapValidator().evaluate(w, h, tiles, caves, emitters, starts);
	EXPECT_GT(rock.cavesMissingPlatform, 0);
	EXPECT_FALSE(rock.valid);
	EXPECT_EQ("cave has no ground, ledge, or bridge below", rock.failureReason);
}

TEST_F(MapValidatorTest, testIntroMoviePackageLayout)
{
	CaveExpressMapContext ctx("intro-movie-package");
	ASSERT_TRUE(ctx.load(false));
	const MapMetrics m = evaluateContext(ctx);
	EXPECT_EQ(1, m.caveCount);
	EXPECT_EQ(0, m.cavesCoveredBySolid) << m.failureReason;
	EXPECT_EQ(m.caveCount, m.cavesReachable) << m.failureReason;
	EXPECT_TRUE(m.valid) << m.failureReason;
}

TEST_F(MapValidatorTest, testGateIsValidFlyablePath)
{
	std::vector<MapTileDefinition> tiles;
	std::vector<CaveTileDefinition> caves;
	std::vector<EmitterDefinition> emitters;
	IMap::StartPositions starts = { { "1", "1" } };
	const int w = 8;
	const int h = 4;
	for (int y = 0; y < h; ++y) {
		for (int x = 0; x < w; ++x) {
			if (x == 0 || y == 0 || x == w - 1 || y == h - 1)
				addTile(tiles, "tile-rock-01", x, y);
			else if (y == h - 2)
				addTile(tiles, "tile-ground-01", x, y);
			else if (!((x == 1 || x == 6) && y == 1))
				addTile(tiles, "tile-background-01", x, y);
		}
	}
	addTile(tiles, "tile-gate-rock-01", 4, 1);
	addTile(tiles, "tile-plate-01-idle", 2, 2);
	const SpriteDefPtr caveDef = requireSprite("tile-cave-01");
	ASSERT_TRUE(!!caveDef);
	caves.emplace_back(1, 1, caveDef, EntityType::NONE, 1000);
	caves.emplace_back(6, 1, caveDef, EntityType::NONE, 1000);

	const MapMetrics m = MapValidator().evaluate(w, h, tiles, caves, emitters, starts);
	EXPECT_EQ(2, m.cavesReachable) << m.failureReason;
	EXPECT_EQ(0, m.unreachableFlyable) << m.failureReason;
	EXPECT_TRUE(m.valid) << m.failureReason;
}

}
