#include "tests/RandomMapContextTest.h"
#include "caveexpress/server/map/RandomMapContext.h"

#if 0
TEST(RandomMapContextTest, testMapCreation)
{
	RandomMapContext ctxIceRandom1("ice-random-1", ThemeType::ICE, 4, 10, 32, 24);
	ASSERT_TRUE(ctxIceRandom1.load(false));

	RandomMapContext ctxRockRandom1("rock-random-1", ThemeType::ROCK, 4, 10, 32, 24);
	ASSERT_TRUE(ctxRockRandom1.load(false));
}

TEST(RandomMapContextTest, testMassMapCreation)
{
	for (int i = 0; i < 100; ++i) {
		const std::string iceFile = String::format("test-random-ice-%i", i);
		RandomMapContext ctxIce(iceFile, ThemeType::ICE, 4, 8, 16, 12);
		ctxIce.load(false);

		const std::string rockFile = String::format("test-random-rock-%i", i);
		RandomMapContext ctxRock(rockFile, ThemeType::ROCK, 4, 8, 16, 12);
		ctxRock.load(false);
	}
}
#endif
