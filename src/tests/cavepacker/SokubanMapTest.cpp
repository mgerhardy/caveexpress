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
		ASSERT_TRUE(map.load(mapName));
		Player* player = new Player(map, 1);
		ASSERT_TRUE(map.initPlayer(player));
	}
}
