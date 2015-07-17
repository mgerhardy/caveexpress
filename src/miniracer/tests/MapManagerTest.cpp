#include "tests/TestShared.h"
#include "common/MapManager.h"

namespace miniracer {

TEST(MapManagerTest, testMapManager)
{
	LUAMapManager mgr;
	mgr.loadMaps();
	ASSERT_FALSE(mgr.getMaps().empty()) << "Could not load any map";
}

}
