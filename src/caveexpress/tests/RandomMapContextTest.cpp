#include "tests/TestShared.h"
#include "caveexpress/server/map/RandomMapContext.h"
#include "caveexpress/server/map/RandomMapGenerator.h"
#include "caveexpress/shared/MapValidator.h"
#include "caveexpress/shared/CaveExpressEntityType.h"
#include "caveexpress/shared/CaveExpressSpriteType.h"
#include "common/ThemeType.h"
#include "common/TextureDefinition.h"
#include "common/SpriteDefinition.h"
#include "common/MapSettings.h"
#include "common/String.h"
#include <cmath>

namespace caveexpress {

class RandomMapContextTest: public AbstractTest {
protected:
	TextureDefinition* _textures = nullptr;

	void SetUp () override
	{
		AbstractTest::SetUp();
		_textures = new TextureDefinition("small");
		SpriteDefinition::get().init(*_textures);
		ASSERT_TRUE(loadEntitySizesFromLua());
	}

	void TearDown () override
	{
		delete _textures;
		_textures = nullptr;
		AbstractTest::TearDown();
	}

	static void expectAcceptedResult (const RandomMapRules& rules, const RandomMapGenerator::Result& result,
			const char* label)
	{
		ASSERT_TRUE(result.success) << label << ": " << result.failureReason;
		ASSERT_FALSE(result.tiles.empty()) << label;
		ASSERT_FALSE(result.startPositions.empty()) << label;
		ASSERT_FALSE(result.caves.empty()) << label;
		const int w = string::toInt(result.settings.at(msn::WIDTH));
		const int h = string::toInt(result.settings.at(msn::HEIGHT));
		const float water = string::toFloat(result.settings.at(msn::WATER_HEIGHT));
		const MapMetrics m = MapValidator().evaluate(w, h, result.tiles, result.caves, result.emitters,
				result.startPositions, rules.minCaveSeparation, rules.minCavePackageAirSeparation,
				rules.minPlatformLength, rules.minSolidComponentSize);
		EXPECT_TRUE(rules.accepts(m, w, h, water)) << label << ": " << m.failureReason;
		EXPECT_EQ(0, m.unreachableFlyable) << label;
		EXPECT_EQ(0, m.cavesAbovePackageTarget) << label;
		EXPECT_EQ(0, m.shortPlatformRuns) << label;
		EXPECT_EQ(0, m.windowWindowAdjacencies) << label;
		EXPECT_EQ(0, m.packageTargetsWithBadNiche) << label;
	}
};

TEST_F(RandomMapContextTest, testRandomMapGeneratorRock)
{
	RandomMapRules rules = RandomMapRules::loadFromLua();
	rules.caveTarget = 3;
	RandomMapGenerator gen(ThemeTypes::ROCK, 20, 14, rules);
	EXPECT_TRUE(gen.gameplayKit().packageTargetRequired);
	EXPECT_EQ(&EntityTypes::PACKAGE_ROCK, gen.gameplayKit().packageEntity);
	const RandomMapGenerator::Result result = gen.generate(42u);
	EXPECT_EQ("3", result.settings.at(msn::PACKAGE_TRANSFER_COUNT));
	EXPECT_EQ("0", result.settings.at(msn::NPC_TRANSFER_COUNT));
	expectAcceptedResult(rules, result, "rock-42");
	EXPECT_NEAR(2.5f, string::toFloat(result.settings.at(msn::WATER_HEIGHT)), 0.01f);
}

TEST_F(RandomMapContextTest, testRandomMapGeneratorIce)
{
	RandomMapRules rules = RandomMapRules::loadFromLua();
	rules.caveTarget = 2;
	RandomMapGenerator gen(ThemeTypes::ICE, 16, 12, rules);
	EXPECT_TRUE(gen.gameplayKit().packageTargetRequired);
	EXPECT_EQ(&EntityTypes::PACKAGE_ICE, gen.gameplayKit().packageEntity);
	expectAcceptedResult(rules, gen.generate(7u), "ice-7");
}

TEST_F(RandomMapContextTest, testAllThemesGenerate)
{
	RandomMapRules rules = RandomMapRules::loadFromLua();
	rules.caveTarget = 2;
	const ThemeType* themes[] = {
		&ThemeTypes::ROCK, &ThemeTypes::ICE, &ThemeTypes::JUNGLE, &ThemeTypes::DESERT
	};
	for (const ThemeType* theme : themes) {
		RandomMapGenerator gen(*theme, 18, 12, rules);
		const RandomMapGenerator::Result result = gen.generate(4242u);
		expectAcceptedResult(rules, result, theme->name.c_str());
		if (theme == &ThemeTypes::JUNGLE || theme == &ThemeTypes::DESERT) {
			EXPECT_FALSE(gen.gameplayKit().packageTargetRequired) << theme->name;
			EXPECT_EQ(0, gen.spriteBucketCounts().packageTargets) << theme->name;
			EXPECT_EQ("0", result.settings.at(msn::PACKAGE_TRANSFER_COUNT)) << theme->name;
			EXPECT_EQ("2", result.settings.at(msn::NPC_TRANSFER_COUNT)) << theme->name;
		} else {
			EXPECT_TRUE(gen.gameplayKit().packageTargetRequired) << theme->name;
			EXPECT_EQ("3", result.settings.at(msn::PACKAGE_TRANSFER_COUNT)) << theme->name;
		}
	}
}

TEST_F(RandomMapContextTest, testMapCreationViaContext)
{
	RandomMapContext ctxIce("ice-random-1", ThemeTypes::ICE, 24, 16);
	ctxIce.setSeed(12345u);
	ctxIce.setCaves(2);
	ASSERT_TRUE(ctxIce.load(false));
	EXPECT_FALSE(ctxIce.getMapTileDefinitions().empty());
	EXPECT_FALSE(ctxIce.getCaveTileDefinitions().empty());
	EXPECT_FALSE(ctxIce.getStartPositions().empty());
	EXPECT_EQ("12345", ctxIce.getSettings().at("seed"));

	RandomMapContext ctxRock("rock-random-1", ThemeTypes::ROCK, 24, 16);
	ctxRock.setSeed(999u);
	ASSERT_TRUE(ctxRock.load(false));
	EXPECT_FALSE(ctxRock.getMapTileDefinitions().empty());
	EXPECT_FALSE(ctxRock.getCaveTileDefinitions().empty());

	RandomMapContext ctxJungle("jungle-random-1", ThemeTypes::JUNGLE, 18, 12);
	ctxJungle.setSeed(77u);
	ASSERT_TRUE(ctxJungle.load(false));
	EXPECT_EQ("0", ctxJungle.getSettings().at(msn::PACKAGE_TRANSFER_COUNT));
	EXPECT_NE("0", ctxJungle.getSettings().at(msn::NPC_TRANSFER_COUNT));
}

TEST_F(RandomMapContextTest, testFacadeWaterMatchesGenerator)
{
	const float water = 1.75f;
	const unsigned int seed = 42u;
	const unsigned int w = 20;
	const unsigned int h = 14;

	RandomMapContext ctx("water-facade", ThemeTypes::ROCK, w, h);
	ctx.setSeed(seed);
	ctx.setCaves(3);
	ctx.setWaterParameters(water, 0.1f, 10000L, 10000L);
	ASSERT_TRUE(ctx.load(false));
	EXPECT_FLOAT_EQ(water, string::toFloat(ctx.getSettings().at(msn::WATER_HEIGHT)));

	RandomMapRules rules = RandomMapRules::loadFromLua();
	rules.caveTarget = 3;
	RandomMapGenerator gen(ThemeTypes::ROCK, w, h, rules, water);
	const RandomMapGenerator::Result result = gen.generate(seed);
	ASSERT_TRUE(result.success);
	EXPECT_FLOAT_EQ(water, string::toFloat(result.settings.at(msn::WATER_HEIGHT)));
	EXPECT_FLOAT_EQ(string::toFloat(result.settings.at(msn::WATER_HEIGHT)),
			string::toFloat(ctx.getSettings().at(msn::WATER_HEIGHT)));
}

TEST_F(RandomMapContextTest, testDeterminismSameSeed)
{
	RandomMapRules rules = RandomMapRules::loadFromLua();
	rules.caveTarget = 2;
	RandomMapGenerator a(ThemeTypes::ROCK, 18, 12, rules);
	RandomMapGenerator b(ThemeTypes::ROCK, 18, 12, rules);
	const auto ra = a.generate(12345u);
	const auto rb = b.generate(12345u);
	ASSERT_TRUE(ra.success);
	ASSERT_TRUE(rb.success);
	ASSERT_EQ(ra.tiles.size(), rb.tiles.size());
	ASSERT_EQ(ra.caves.size(), rb.caves.size());
	ASSERT_EQ(ra.emitters.size(), rb.emitters.size());
	ASSERT_EQ(ra.startPositions.size(), rb.startPositions.size());
	for (size_t i = 0; i < ra.tiles.size(); ++i) {
		EXPECT_FLOAT_EQ(ra.tiles[i].x, rb.tiles[i].x) << i;
		EXPECT_FLOAT_EQ(ra.tiles[i].y, rb.tiles[i].y) << i;
		ASSERT_TRUE(!!ra.tiles[i].spriteDef && !!rb.tiles[i].spriteDef) << i;
		EXPECT_EQ(ra.tiles[i].spriteDef->id, rb.tiles[i].spriteDef->id) << i;
	}
}

TEST_F(RandomMapContextTest, testFixedSeedsMeetAcceptanceProperties)
{
	RandomMapRules rules = RandomMapRules::loadFromLua();
	rules.caveTarget = 2;
	const unsigned int seeds[] = { 42u, 7u, 4242u, 1000u, 3000u, 5555u, 9999u, 1234u };
	int failures = 0;
	for (unsigned int seed : seeds) {
		for (const ThemeType* theme : { &ThemeTypes::ROCK, &ThemeTypes::ICE, &ThemeTypes::JUNGLE, &ThemeTypes::DESERT }) {
			RandomMapGenerator gen(*theme, 18, 12, rules);
			const RandomMapGenerator::Result result = gen.generate(seed);
			if (!result.success) {
				++failures;
				ADD_FAILURE() << theme->name << " seed " << seed << ": " << result.failureReason;
				continue;
			}
			const int w = string::toInt(result.settings.at(msn::WIDTH));
			const int h = string::toInt(result.settings.at(msn::HEIGHT));
			const MapMetrics m = MapValidator().evaluate(w, h, result.tiles, result.caves, result.emitters,
					result.startPositions, rules.minCaveSeparation, rules.minCavePackageAirSeparation,
					rules.minPlatformLength, rules.minSolidComponentSize);
			EXPECT_TRUE(rules.accepts(m, w, h, string::toFloat(result.settings.at(msn::WATER_HEIGHT))))
					<< theme->name << " seed " << seed << ": " << m.failureReason;
		}
	}
	EXPECT_EQ(0, failures);
}

TEST_F(RandomMapContextTest, testRandomMapFullAirReachability)
{
	RandomMapRules rules = RandomMapRules::loadFromLua();
	rules.caveTarget = 2;
	RandomMapGenerator gen(ThemeTypes::ROCK, 18, 12, rules);
	const RandomMapGenerator::Result result = gen.generate(4242u);
	expectAcceptedResult(rules, result, "reach-4242");
	const int w = string::toInt(result.settings.at(msn::WIDTH));
	const int h = string::toInt(result.settings.at(msn::HEIGHT));
	const MapMetrics m = MapValidator().evaluate(w, h, result.tiles, result.caves, result.emitters,
			result.startPositions, rules.minCaveSeparation, rules.minCavePackageAirSeparation,
			rules.minPlatformLength, rules.minSolidComponentSize);
	EXPECT_FLOAT_EQ(1.0f, m.flyableReachabilityRatio);
}

TEST_F(RandomMapContextTest, testWalkablesStayAboveWaterline)
{
	RandomMapRules rules = RandomMapRules::loadFromLua();
	rules.caveTarget = 2;
	const float waters[] = { 1.0f, 2.0f, 2.5f };
	const unsigned int seeds[] = { 42u, 7u, 4242u };
	for (float water : waters) {
		for (unsigned int seed : seeds) {
			RandomMapGenerator gen(ThemeTypes::ROCK, 20, 14, rules, water);
			const RandomMapGenerator::Result result = gen.generate(seed);
			ASSERT_TRUE(result.success) << "water=" << water << " seed=" << seed << " " << result.failureReason;
			const float surface = 14.0f - water;
			const float maxWalkTop = surface - 1.0f;
			for (const MapTileDefinition& tile : result.tiles) {
				if (!tile.spriteDef)
					continue;
				const SpriteType& type = tile.spriteDef->type;
				if (!SpriteTypes::isAnyGround(type) && !SpriteTypes::isBridge(type) && !SpriteTypes::isSlope(type))
					continue;
				EXPECT_LE(tile.y, maxWalkTop + 1e-3f)
						<< "water=" << water << " seed=" << seed << " tile=" << tile.spriteDef->id
						<< " y=" << tile.y << " surface=" << surface;
			}
		}
	}
}

TEST_F(RandomMapContextTest, testEntitySizesLoadedFromLua)
{
	EXPECT_FLOAT_EQ(2.0f, EntityTypes::TREE.width);
	EXPECT_FLOAT_EQ(2.0f, EntityTypes::TREE.height);
	EXPECT_FLOAT_EQ(1.95f, EntityTypes::NPC_WALKING.width);
	EXPECT_FLOAT_EQ(0.8f, EntityTypes::NPC_WALKING.height);
}

TEST_F(RandomMapContextTest, testCaveArtPoolDistinctFromBackgrounds)
{
	RandomMapGenerator rock(ThemeTypes::ROCK, 18, 12);
	EXPECT_GT(rock.spriteBucketCounts().backgrounds, 0);
	EXPECT_GT(rock.spriteBucketCounts().caveArt, 0);
	EXPECT_LT(rock.spriteBucketCounts().caveArt, rock.spriteBucketCounts().backgrounds);

	RandomMapGenerator jungle(ThemeTypes::JUNGLE, 18, 12);
	EXPECT_GT(jungle.spriteBucketCounts().caveArt, 0);
}

TEST_F(RandomMapContextTest, testAcceptsGridStructuralGates)
{
	RandomMapRules rules;
	rules.minSurfaceCells = 6;
	rules.minAirPercent = 25;
	EXPECT_FALSE(rules.acceptsGrid(5, 100, 200));
	EXPECT_FALSE(rules.acceptsGrid(6, 40, 200));
	EXPECT_TRUE(rules.acceptsGrid(6, 50, 200));
}

TEST_F(RandomMapContextTest, testPropagateRequeuesOnNarrowing)
{
	RandomMapGenerator gen(ThemeTypes::ROCK, 10, 8);
	const uint16_t all = static_cast<uint16_t>((1u << static_cast<unsigned>(RandomMapGenerator::Cell::Count)) - 1u);
	std::vector<uint16_t> domain(10 * 8, all);
	const int x = 2;
	const int y = 3;
	const int above = x + (y - 1) * 10;
	domain[x + y * 10] = RandomMapGenerator::cellBit(RandomMapGenerator::Cell::Shim);
	ASSERT_TRUE(gen.propagateDomain(domain, x, y));
	EXPECT_EQ(0, domain[above] & RandomMapGenerator::cellBit(RandomMapGenerator::Cell::Air));
	EXPECT_EQ(0, domain[above] & RandomMapGenerator::cellBit(RandomMapGenerator::Cell::Ground));
	EXPECT_NE(0u, domain[above] & RandomMapGenerator::cellBit(RandomMapGenerator::Cell::Rock));
}

TEST_F(RandomMapContextTest, testCompatibleAdjacencyMatrix)
{
	using Cell = RandomMapGenerator::Cell;
	RandomMapGenerator gen(ThemeTypes::ROCK, 10, 8);
	struct Case {
		Cell a;
		Cell b;
		int dir;
		bool ok;
		const char* name;
	};
	const Case cases[] = {
		{ Cell::Shim, Cell::Rock, 0, true, "shim-N-rock" },
		{ Cell::Shim, Cell::Air, 0, false, "shim-N-air" },
		{ Cell::Shim, Cell::Air, 2, true, "shim-S-air" },
		{ Cell::Shim, Cell::Rock, 2, false, "shim-S-rock" },
		{ Cell::Ground, Cell::Air, 0, true, "ground-N-air" },
		{ Cell::Ground, Cell::Rock, 0, false, "ground-N-rock" },
		{ Cell::Bridge, Cell::Air, 0, true, "bridge-N-air" },
		{ Cell::Bridge, Cell::Rock, 0, false, "bridge-N-rock" },
		{ Cell::Bridge, Cell::Ground, 1, true, "bridge-E-ground" },
		{ Cell::Bridge, Cell::Rock, 1, false, "bridge-E-rock" },
		{ Cell::Rock, Cell::Bridge, 1, false, "rock-E-bridge" },
		{ Cell::Rock, Cell::Air, 2, true, "rock-S-air" },
		{ Cell::UndercutL, Cell::Rock, 0, true, "undercutL-N-rock" },
		{ Cell::UndercutL, Cell::Air, 2, true, "undercutL-S-air" },
		{ Cell::UndercutL, Cell::Air, 3, true, "undercutL-W-air" },
		{ Cell::UndercutL, Cell::Rock, 1, true, "undercutL-E-rock" },
		{ Cell::UndercutL, Cell::Air, 1, false, "undercutL-E-air" },
		{ Cell::UndercutR, Cell::Air, 1, true, "undercutR-E-air" },
		{ Cell::UndercutR, Cell::Rock, 3, true, "undercutR-W-rock" },
		{ Cell::LedgeL, Cell::Air, 2, true, "ledgeL-S-air" },
		{ Cell::LedgeL, Cell::Rock, 2, false, "ledgeL-S-rock" },
		{ Cell::LedgeR, Cell::Air, 1, true, "ledgeR-E-air" },
		{ Cell::SlopeL, Cell::Air, 0, true, "slopeL-N-air" },
		{ Cell::SlopeL, Cell::Rock, 2, true, "slopeL-S-rock" },
		{ Cell::SlopeR, Cell::Air, 0, true, "slopeR-N-air" },
		{ Cell::Air, Cell::Air, 1, true, "air-E-air" },
		{ Cell::Air, Cell::Rock, 1, true, "air-E-rock" },
	};
	for (const Case& c : cases)
		EXPECT_EQ(c.ok, gen.cellsCompatible(c.a, c.b, c.dir)) << c.name;

	struct Bi {
		Cell a;
		Cell b;
		int dir;
		const char* name;
	};
	const Bi bidirectional[] = {
		{ Cell::Shim, Cell::Rock, 0, "shim-rock-N" },
		{ Cell::Ground, Cell::Air, 0, "ground-air-N" },
		{ Cell::UndercutL, Cell::Air, 2, "undercutL-air-S" },
		{ Cell::Bridge, Cell::Ground, 1, "bridge-ground-E" },
	};
	for (const Bi& b : bidirectional) {
		const int opposite = (b.dir + 2) % 4;
		EXPECT_TRUE(gen.cellsCompatible(b.a, b.b, b.dir)) << b.name;
		EXPECT_TRUE(gen.cellsCompatible(b.b, b.a, opposite)) << b.name << " reverse";
	}
}

TEST_F(RandomMapContextTest, testObserveSkipsEmptyUndercutShimBuckets)
{
	RandomMapGenerator gen(ThemeTypes::ROCK, 10, 8);
	ASSERT_GT(gen.spriteBucketCounts().shims, 0);
	gen.clearUndercutShimBucketsForTest();
	EXPECT_EQ(0, gen.spriteBucketCounts().shims);

	const uint16_t mask = static_cast<uint16_t>(
			RandomMapGenerator::cellBit(RandomMapGenerator::Cell::UndercutL)
			| RandomMapGenerator::cellBit(RandomMapGenerator::Cell::UndercutR)
			| RandomMapGenerator::cellBit(RandomMapGenerator::Cell::Shim)
			| RandomMapGenerator::cellBit(RandomMapGenerator::Cell::Rock)
			| RandomMapGenerator::cellBit(RandomMapGenerator::Cell::Air));
	for (unsigned int seed = 1; seed < 200; ++seed) {
		unsigned int rng = seed;
		const RandomMapGenerator::Cell c = gen.observeForTest(mask, rng);
		EXPECT_TRUE(c == RandomMapGenerator::Cell::Rock || c == RandomMapGenerator::Cell::Air)
				<< "seed " << seed << " got " << static_cast<int>(c);
	}
}

}
