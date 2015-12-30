#include "tests/TestShared.h"
#include "caveexpress/shared/CaveExpressEntityType.h"
#include "caveexpress/shared/CaveExpressAnimation.h"
#include "caveexpress/shared/CaveExpressSpriteType.h"
#include "common/Log.h"
#include "common/SpriteDefinition.h"
#include "common/TextureDefinition.h"
#include "ui/FontDefinition.h"
#include "common/EntityType.h"
#include "common/Animation.h"

namespace caveexpress {

TEST(MapManagerTest, testLoad)
{
	LUAMapManager mgr;
	mgr.loadMaps();
	ASSERT_FALSE(mgr.getMaps().empty());
}

TEST(MapManagerTest, testMeta)
{
	LUAMapManager mgr;
	mgr.loadMaps();
	ASSERT_EQ("Package", mgr.getMapTitle("introducing-01-package"));
	//ASSERT_EQ(1, mgr.getMapStartPositions("introducing-01-package"));

	ASSERT_EQ("Be fast", mgr.getMapTitle("introducing-02-game"));
	//ASSERT_EQ(1, mgr.getMapStartPositions("introducing-02-game"));

	ASSERT_EQ("Tree", mgr.getMapTitle("introducing-03-tree"));
	//ASSERT_EQ(1, mgr.getMapStartPositions("introducing-03-tree"));
}

TEST(MapManagerTest, testStartPositions)
{
	LUAMapManager mgr;
	mgr.loadMaps();
	const IMapManager::Maps& maps = mgr.getMaps();
	for (auto entry : maps) {
		if (string::startsWith(entry.first, "test") || string::startsWith(entry.first, "empty"))
			continue;
		ASSERT_NE("", mgr.getMapTitle(entry.first)) << entry.first << " has no title set";
		ASSERT_GE(mgr.getMapStartPositions(entry.first), 1) << entry.first << " has no start positions set";
	}
}

}
