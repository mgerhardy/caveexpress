#include "MapManagerTest.h"
#include "cavepacker/shared/CavePackerMapManager.h"

TEST(MapManagerTest, testNew) {
	CavePackerMapManager p;
	p.loadMaps();
	ASSERT_EQ(2, p.getMapStartPositions("multiplayer0001"));
	ASSERT_EQ(1, p.getMapStartPositions("tutorial0001"));
}
