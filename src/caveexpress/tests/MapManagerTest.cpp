#include "tests/TestShared.h"
#include "caveexpress/shared/CaveExpressEntityType.h"
#include "caveexpress/shared/CaveExpressAnimation.h"
#include "caveexpress/shared/CaveExpressSpriteType.h"
#include "caveexpress/shared/CaveExpressMapContext.h"
#include "common/Log.h"
#include "common/SpriteDefinition.h"
#include "common/TextureDefinition.h"
#include "common/FileSystem.h"
#include "common/MapManager.h"
#include "common/StartPositionMode.h"
#include "ui/FontDefinition.h"
#include "common/EntityType.h"
#include "common/Animation.h"
#include <cstring>
#include <fstream>

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
	ASSERT_EQ("In1 Package", mgr.getMapTitle("introducing-01-package"));
	ASSERT_EQ(1, mgr.getMapStartPositions("introducing-01-package"));

	ASSERT_EQ("In2 Be fast", mgr.getMapTitle("introducing-02-game"));
	ASSERT_EQ(1, mgr.getMapStartPositions("introducing-02-game"));

	ASSERT_EQ("In3 Tree", mgr.getMapTitle("introducing-03-tree"));
	ASSERT_EQ(1, mgr.getMapStartPositions("introducing-03-tree"));

	ASSERT_EQ("In12 Lava", mgr.getMapTitle("introducing-12-lava"));
	ASSERT_EQ("In13 Water rising", mgr.getMapTitle("introducing-13-waterrising"));
	ASSERT_EQ("In14 Fish", mgr.getMapTitle("introducing-14-fish"));
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

TEST(MapManagerTest, testStartPositionModesPersist)
{
	const char* lua =
		"function getName()\n"
		"\treturn \"Start modes\"\n"
		"end\n"
		"function initMap()\n"
		"\tlocal map = Map.get()\n"
		"\tmap:addStartPosition(\"1\", \"2\")\n"
		"\tmap:addStartPosition(\"8\", \"3\", \"multiplayer\")\n"
		"end\n";
	const std::string name = "start_modes_lua";
	const std::string relPath = FS.getDataDir() + FS.getMapsDir() + name + ".lua";
	const std::string absPath = FS.getAbsoluteWritePath() + relPath;
	ASSERT_NE(-1L, FS.writeSysFile(absPath, (const unsigned char*)lua, strlen(lua), true));

	CaveExpressMapContext ctx(name);
	ASSERT_TRUE(ctx.load(false));
	const IMap::StartPositions& starts = ctx.getStartPositions();
	ASSERT_EQ(2u, starts.size());
	EXPECT_EQ(StartPositionModes::BOTH, starts[0]._mode);
	EXPECT_EQ(StartPositionModes::MULTIPLAYER, starts[1]._mode);
	EXPECT_EQ(2, StartPositionModes::countMultiplayerStartsFromLua(lua));

	const std::string outName = "start_modes_lua_out";
	const std::string outRel = FS.getDataDir() + FS.getMapsDir() + outName + ".lua";
	const std::string outAbs = FS.getAbsoluteWritePath() + outRel;
	ASSERT_TRUE(ctx.saveToPath(outAbs));

	std::ifstream in(outAbs.c_str(), std::ios::binary);
	ASSERT_TRUE(in.good());
	const std::string saved((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
	EXPECT_NE(std::string::npos, saved.find("multiplayer"));
	EXPECT_EQ(std::string::npos, saved.find("singleplayer"));

	CaveExpressMapContext reloaded(outName);
	ASSERT_TRUE(reloaded.load(false));
	ASSERT_EQ(2u, reloaded.getStartPositions().size());
	EXPECT_EQ(StartPositionModes::BOTH, reloaded.getStartPositions()[0]._mode);
	EXPECT_EQ(StartPositionModes::MULTIPLAYER, reloaded.getStartPositions()[1]._mode);

	FS.deleteFile(relPath);
	FS.deleteFile(outRel);
}

}
