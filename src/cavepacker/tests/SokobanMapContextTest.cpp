#include "tests/TestShared.h"
#include "cavepacker/server/map/SokobanMapContext.h"
#include "cavepacker/shared/CavePackerSpriteType.h"
#include "common/FileSystem.h"
#include "common/SpriteDefinition.h"
#include "common/TextureDefinition.h"
#include <cstring>

namespace cavepacker {

class SokobanMapContextTest: public AbstractTest {
protected:
	void SetUp() override {
		AbstractTest::SetUp();
		TextureDefinition t("small");
		SpriteDefinition::get().init(t);
	}

	static bool hasTileAt(const SokobanMapContext& ctx, int col, int row) {
		for (const MapTileDefinition& tile : ctx.getMapTileDefinitions()) {
			if (tile.x == col && tile.y == row)
				return true;
		}
		return false;
	}

	static bool hasGroundAt(const SokobanMapContext& ctx, int col, int row) {
		for (const MapTileDefinition& tile : ctx.getMapTileDefinitions()) {
			if (tile.x == col && tile.y == row && SpriteTypes::isGround(tile.spriteDef->type))
				return true;
		}
		return false;
	}
};

TEST_F(SokobanMapContextTest, testMapCreation)
{
	for (int i = 1; i <= 90; ++i) {
		const std::string name = string::format("xsokoban%04i", i);
		SokobanMapContext ctx(name);
		ASSERT_TRUE(ctx.load(false));
	}
}

TEST_F(SokobanMapContextTest, testTrailingVoidDoesNotBecomeGround)
{
	// Leading void, walls, trailing void on the same row must not spawn floor tiles
	// on the right-hand empty slots (common after editor save of a non-full-width board).
	const char* board =
		";trailing-void-test\n"
		"  #####  \n"
		"  # @ #  \n"
		"  # $ #  \n"
		"  # . #  \n"
		"  #####  \n";
	const std::string name = "trailing_void_test";
	const std::string relPath = FS.getDataDir() + FS.getMapsDir() + name + ".sok";
	const std::string absPath = FS.getAbsoluteWritePath() + relPath;
	ASSERT_NE(-1L, FS.writeSysFile(absPath, (const unsigned char*)board, strlen(board), true))
		<< "Failed to write " << absPath;

	SokobanMapContext ctx(name);
	ASSERT_TRUE(ctx.load(false));

	// Left padding (cols 0-1) and right padding (cols 7-8) stay empty on every row.
	for (int row = 0; row < 5; ++row) {
		EXPECT_FALSE(hasTileAt(ctx, 0, row)) << "left void filled at 0," << row;
		EXPECT_FALSE(hasTileAt(ctx, 1, row)) << "left void filled at 1," << row;
		EXPECT_FALSE(hasTileAt(ctx, 7, row)) << "right void filled at 7," << row;
		EXPECT_FALSE(hasTileAt(ctx, 8, row)) << "right void filled at 8," << row;
	}

	// Interior floors between walls (and under player/package) must still exist.
	EXPECT_TRUE(hasGroundAt(ctx, 3, 1));
	EXPECT_TRUE(hasGroundAt(ctx, 4, 1));
	EXPECT_TRUE(hasGroundAt(ctx, 5, 1));
	EXPECT_TRUE(hasGroundAt(ctx, 3, 2));
	EXPECT_TRUE(hasGroundAt(ctx, 4, 2));
	EXPECT_TRUE(hasGroundAt(ctx, 5, 2));
	EXPECT_TRUE(hasGroundAt(ctx, 3, 3));
	EXPECT_TRUE(hasGroundAt(ctx, 5, 3));

	FS.deleteFile(relPath);
}

}
