#include "tests/TestShared.h"
#include "caveexpress/server/map/RandomMapContext.h"
#include "caveexpress/server/map/WfcMapGenerator.h"
#include "common/ThemeType.h"
#include "common/TextureDefinition.h"
#include "common/SpriteDefinition.h"
#include "common/MapSettings.h"

namespace caveexpress {

class RandomMapContextTest: public AbstractTest {
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
};

TEST_F(RandomMapContextTest, testWfcGeneratorRock)
{
	WfcMapGenerator gen(ThemeTypes::ROCK, 20, 14, 3);
	const WfcMapGenerator::Result result = gen.generate(42u);
	ASSERT_TRUE(result.success);
	EXPECT_FALSE(result.tiles.empty());
	EXPECT_FALSE(result.startPositions.empty());
	EXPECT_FALSE(result.caves.empty());
	EXPECT_EQ("20", result.settings.at(msn::WIDTH));
	EXPECT_EQ("14", result.settings.at(msn::HEIGHT));
}

TEST_F(RandomMapContextTest, testWfcGeneratorIce)
{
	WfcMapGenerator gen(ThemeTypes::ICE, 16, 12, 2);
	const WfcMapGenerator::Result result = gen.generate(7u);
	ASSERT_TRUE(result.success);
	EXPECT_FALSE(result.tiles.empty());
	EXPECT_FALSE(result.caves.empty());
}

TEST_F(RandomMapContextTest, testMapCreationViaContext)
{
	srand(12345);
	RandomMapContext ctxIce("ice-wfc-1", ThemeTypes::ICE, 4, 10, 24, 16);
	ASSERT_TRUE(ctxIce.load(false));
	EXPECT_FALSE(ctxIce.getMapTileDefinitions().empty());
	EXPECT_FALSE(ctxIce.getCaveTileDefinitions().empty());
	EXPECT_FALSE(ctxIce.getStartPositions().empty());

	RandomMapContext ctxRock("rock-wfc-1", ThemeTypes::ROCK, 4, 10, 24, 16);
	ASSERT_TRUE(ctxRock.load(false));
	EXPECT_FALSE(ctxRock.getMapTileDefinitions().empty());
	EXPECT_FALSE(ctxRock.getCaveTileDefinitions().empty());
}

TEST_F(RandomMapContextTest, testMassWfcCreation)
{
	int successes = 0;
	for (int i = 0; i < 40; ++i) {
		WfcMapGenerator gen((i & 1) ? ThemeTypes::ICE : ThemeTypes::ROCK, 16, 12, 2);
		if (gen.generate(static_cast<unsigned int>(1000 + i)).success)
			++successes;
	}
	EXPECT_GE(successes, 30);
}

}
