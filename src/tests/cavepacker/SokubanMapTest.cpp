#include "tests/cavepacker/SokubanMapTest.h"
#include "cavepacker/server/map/Map.h"
#include "cavepacker/server/entities/Player.h"

class SokubanMapTest: public MapSuite {
protected:
	void testMapRegion (int begin)
	{
		for (int i = begin; i < begin + 10; ++i) {
			Map map;
			map.init(&_testFrontend, _serviceProvider);
			std::string mapName = String::format("%04i", i);
			ASSERT_TRUE(map.load(mapName)) << "Failed to load map " << mapName;
			Player* player = new Player(map, 1);
			ASSERT_TRUE(map.initPlayer(player)) << "Failed to init the player for map " << mapName;
			map.startMap();
			const int steps = map.solve();
			ASSERT_TRUE(map.isAutoSolve()) << "Map " << mapName << " is not in autoSolve mode";
			ASSERT_TRUE(steps >= 1) << "No step in solution for map " << mapName;
			for (int s = 0; s < steps; ++s) {
				map.update(10000);
				ASSERT_TRUE(map.isAutoSolve()) << "Map " << mapName << " is no longer in autoSolve mode in step " << s;
			}
			EXPECT_TRUE(map.isDone()) << "Autosolve for map " << mapName << " did not lead to a done map";
		}
	}
};

TEST_F(SokubanMapTest, testMaps10)
{
	testMapRegion(1);
}

TEST_F(SokubanMapTest, testMaps20)
{
	testMapRegion(11);
}

TEST_F(SokubanMapTest, testMaps30)
{
	testMapRegion(21);
}

TEST_F(SokubanMapTest, testMaps40)
{
	testMapRegion(31);
}

TEST_F(SokubanMapTest, testMaps50)
{
	testMapRegion(41);
}

TEST_F(SokubanMapTest, testMaps60)
{
	testMapRegion(51);
}

TEST_F(SokubanMapTest, testMaps70)
{
	testMapRegion(61);
}

TEST_F(SokubanMapTest, testMaps80)
{
	testMapRegion(71);
}

TEST_F(SokubanMapTest, testMaps90)
{
	testMapRegion(81);
}
