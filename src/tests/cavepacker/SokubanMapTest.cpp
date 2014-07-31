#include "tests/cavepacker/SokubanMapTest.h"
#include "cavepacker/server/map/Map.h"
#include "cavepacker/server/entities/Player.h"

class SokubanMapTest: public MapSuite {
};

TEST_F(SokubanMapTest, testMaps)
{
	for (int i = 1; i <= 90; ++i) {
		Map map;
		map.init(&_testFrontend, _serviceProvider);
		std::string mapName = String::format("%04i", i);
		SCOPED_TRACE(mapName);
		ASSERT_TRUE(map.load(mapName));
		Player* player = new Player(map, 1);
		ASSERT_TRUE(map.initPlayer(player));
		const int steps = map.solve();
		EXPECT_TRUE(steps >= 1);
		for (int s = 0; s < steps; ++s) {
			map.update(10000);
			EXPECT_TRUE(map.isAutoSolve());
		}
		EXPECT_TRUE(map.isDone());
	}
}
