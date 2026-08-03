#include "tests/TestShared.h"
#include "caveexpress/shared/MapValidator.h"
#include "caveexpress/shared/CaveExpressMapContext.h"
#include "caveexpress/server/map/RandomMapGenerator.h"
#include "common/ThemeType.h"
#include "common/TextureDefinition.h"
#include "common/SpriteDefinition.h"
#include "common/MapSettings.h"
#include "common/Log.h"
#include "common/String.h"
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
		return MapValidator().evaluate(w, h, ctx.getMapTileDefinitions(), ctx.getCaveTileDefinitions(),
				ctx.getEmitterDefinitions(), ctx.getStartPositions());
	}

	SpriteDefPtr requireSprite (const char* id) const
	{
		const SpriteDefPtr def = SpriteDefinition::get().getSpriteDefinition(id);
		EXPECT_TRUE(!!def) << id;
		return def;
	}

	void addTile (std::vector<MapTileDefinition>& tiles, const char* id, int x, int y) const
	{
		const SpriteDefPtr def = requireSprite(id);
		if (def)
			tiles.emplace_back(static_cast<gridCoord>(x), static_cast<gridCoord>(y), def, 0);
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

TEST_F(MapValidatorTest, testHandMapBaseline)
{
	const char* maps[] = {
		"rock-01", "rock-08", "wind-01", "wind-02", "wind-03",
		"villages-01", "villages-08", "third-ice-01", "third-ice-05"
	};

	int hardPass = 0;
	for (const char* name : maps) {
		CaveExpressMapContext ctx(name);
		ASSERT_TRUE(ctx.load(true)) << name;
		const MapMetrics m = evaluateContext(ctx);
		Log::info(LOG_GAMEIMPL,
				"hand map %s score=%.1f valid=%i exposed=%.3f orphan=%.3f caves=%i/%i",
				name, m.totalScore, m.valid ? 1 : 0, m.exposedRockTopRatio, m.orphanColliderRatio,
				m.cavesReachable, m.caveCount);
		if (m.valid)
			++hardPass;
		EXPECT_TRUE(m.valid || m.caveCount == 0) << name << ": " << m.failureReason;
	}
	EXPECT_GE(hardPass, static_cast<int>(sizeof(maps) / sizeof(maps[0])) - 2);
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

}
