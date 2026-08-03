#include "tests/TestShared.h"
#include "cavepacker/server/map/SokobanMapContext.h"
#include "cavepacker/shared/CavePackerSpriteType.h"
#include "cavepacker/shared/WallTilePlacement.h"
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

	static std::string rockAt(const SokobanMapContext& ctx, int col, int row) {
		for (const MapTileDefinition& tile : ctx.getMapTileDefinitions()) {
			if (tile.x == col && tile.y == row && SpriteTypes::isSolid(tile.spriteDef->type))
				return tile.spriteDef->id;
		}
		return "";
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

TEST_F(SokobanMapContextTest, testWallPlacementCompute)
{
	EXPECT_EQ(WallPlacement::Down, computeWallPlacement(false, false, false, true, true, true, true, false));
	EXPECT_EQ(WallPlacement::Left, computeWallPlacement(true, false, false, false, false, true, true, true));
	EXPECT_EQ(WallPlacement::Right, computeWallPlacement(false, true, false, false, true, false, true, true));
	EXPECT_EQ(WallPlacement::Top, computeWallPlacement(false, false, true, false, true, true, false, true));
	EXPECT_EQ(WallPlacement::Full, computeWallPlacement(false, false, false, false, true, true, true, true));
	// corner with two opens -> any
	EXPECT_EQ(WallPlacement::Any, computeWallPlacement(false, true, false, true, true, false, true, false));
	// exterior edge (no playable, not fully walled) -> any
	EXPECT_EQ(WallPlacement::Any, computeWallPlacement(false, false, false, false, false, true, true, true));
}

TEST_F(SokobanMapContextTest, testDownPlacementTilesOnlyOnDownEdges)
{
	ASSERT_EQ("down", SpriteDefinition::get().getSpriteDefinition("tile-rock-04")->placement);
	ASSERT_EQ("down", SpriteDefinition::get().getSpriteDefinition("tile-rock-05")->placement);
	ASSERT_EQ("any", SpriteDefinition::get().getSpriteDefinition("tile-rock-01")->placement);

	// Force many loads; down-only rocks must never appear on a pure left-edge wall.
	const char* board =
		";wall-placement-test\n"
		"#####\n"
		"# @ #\n"
		"# $ #\n"
		"# . #\n"
		"#####\n";
	const std::string name = "wall_placement_test";
	const std::string relPath = FS.getDataDir() + FS.getMapsDir() + name + ".sok";
	const std::string absPath = FS.getAbsoluteWritePath() + relPath;
	ASSERT_NE(-1L, FS.writeSysFile(absPath, (const unsigned char*)board, strlen(board), true));

	int caveCount = 0;
	int torchCount = 0;
	for (int n = 0; n < 40; ++n) {
		SokobanMapContext ctx(name);
		ASSERT_TRUE(ctx.load(false));
		// Left wall column 0, rows 1-3: open only to the right -> not "down"
		for (int row = 1; row <= 3; ++row) {
			const std::string id = rockAt(ctx, 0, row);
			EXPECT_TRUE(id == "tile-rock-01" || id == "tile-rock-02" || id == "tile-rock-03")
				<< "unexpected rock " << id << " on left edge at row " << row;
		}
		// Right wall column 4, rows 1-3
		for (int row = 1; row <= 3; ++row) {
			const std::string id = rockAt(ctx, 4, row);
			EXPECT_TRUE(id == "tile-rock-01" || id == "tile-rock-02" || id == "tile-rock-03")
				<< "unexpected rock " << id << " on right edge at row " << row;
		}
		// Top wall (row 0, cols 1-3): open below -> down-only pool (cave/torch)
		for (int col = 1; col <= 3; ++col) {
			const std::string id = rockAt(ctx, col, 0);
			EXPECT_TRUE(id == "tile-rock-04" || id == "tile-rock-05")
				<< "expected cave/torch on down edge, got " << id;
			if (id == "tile-rock-04")
				++caveCount;
			else if (id == "tile-rock-05")
				++torchCount;
		}
	}

	EXPECT_GT(caveCount, 0) << "tile-rock-04 (cave) was never selected on down edges";
	EXPECT_GT(torchCount, 0) << "tile-rock-05 (torch) was never selected on down edges";

	FS.deleteFile(relPath);
}

}
