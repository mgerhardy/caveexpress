#include "tests/TestShared.h"
#include "caveexpress/client/editor/MapEditorDocument.h"
#include "common/MapManager.h"
#include "common/SpriteDefinition.h"
#include "common/TextureDefinition.h"
#include "common/ThemeType.h"
#include "common/vec2.h"
#include <algorithm>

namespace caveexpress {

class MapEditorDocumentTest: public AbstractTest {
protected:
	TextureDefinition* _textures = nullptr;
	LUAMapManager _mapMgr;

	void SetUp () override
	{
		AbstractTest::SetUp();
		_textures = new TextureDefinition("small");
		SpriteDefinition::get().init(*_textures);
	}

	void TearDown () override
	{
		delete _textures;
		_textures = nullptr;
		AbstractTest::TearDown();
	}

	SpriteDefPtr requireSprite (const char* id) const
	{
		const SpriteDefPtr def = SpriteDefinition::get().getSpriteDefinition(id);
		EXPECT_TRUE(!!def) << id;
		return def;
	}

	int countSprite (const MapEditorDocument& doc, const char* id) const
	{
		int n = 0;
		for (const MapEditorTileItem& item : doc.getTiles()) {
			if (item.def && item.def->id == id)
				++n;
		}
		return n;
	}

	bool hasSpriteAt (const MapEditorDocument& doc, const char* id, gridCoord x, gridCoord y) const
	{
		for (const MapEditorTileItem& item : doc.getTiles()) {
			if (item.def && item.def->id == id && fequals(item.gridX, x) && fequals(item.gridY, y))
				return true;
		}
		return false;
	}
};

TEST_F(MapEditorDocumentTest, testPlaceLianeOnOneCellCorridor)
{
	MapEditorDocument doc(_mapMgr);
	const SpriteDefPtr background = requireSprite("tile-background-01");
	const SpriteDefPtr ground = requireSprite("tile-ground-01");
	const SpriteDefPtr liane = requireSprite("liane-01");
	ASSERT_TRUE(!!background && !!ground && !!liane);

	doc.setSprite(background);
	doc.setSelectedGrid(2.0f, 3.0f);
	ASSERT_TRUE(doc.paintAtSelection(true, false));

	doc.setSprite(ground);
	doc.setSelectedGrid(2.0f, 4.0f);
	ASSERT_TRUE(doc.paintAtSelection(true, false));

	doc.setSprite(liane);
	doc.setSelectedGrid(2.0f, 3.0f);
	ASSERT_TRUE(doc.paintAtSelection(true, false));
	EXPECT_EQ(1, countSprite(doc, "liane-01"));
	EXPECT_TRUE(hasSpriteAt(doc, "tile-ground-01", 2.0f, 4.0f));
	EXPECT_TRUE(hasSpriteAt(doc, "tile-background-01", 2.0f, 3.0f));
}

TEST_F(MapEditorDocumentTest, testPlaceLianeOverWindow)
{
	MapEditorDocument doc(_mapMgr);
	const SpriteDefPtr background = requireSprite("tile-background-01");
	const SpriteDefPtr window = requireSprite("tile-background-window-02");
	const SpriteDefPtr liane = requireSprite("liane-01");
	ASSERT_TRUE(!!background && !!window && !!liane);

	doc.setSprite(background);
	doc.setSelectedGrid(1.0f, 6.0f);
	ASSERT_TRUE(doc.paintAtSelection(true, false));
	doc.setSprite(window);
	doc.setSelectedGrid(1.0f, 7.0f);
	ASSERT_TRUE(doc.paintAtSelection(true, false));

	doc.setSprite(liane);
	doc.setSelectedGrid(1.0f, 6.0f);
	ASSERT_TRUE(doc.paintAtSelection(true, false));
	EXPECT_EQ(1, countSprite(doc, "liane-01"));
	EXPECT_TRUE(hasSpriteAt(doc, "tile-background-window-02", 1.0f, 7.0f));
}

TEST_F(MapEditorDocumentTest, testRejectLianeOnSolid)
{
	MapEditorDocument doc(_mapMgr);
	const SpriteDefPtr rock = requireSprite("tile-rock-01");
	const SpriteDefPtr liane = requireSprite("liane-01");
	ASSERT_TRUE(!!rock && !!liane);

	doc.setSprite(rock);
	doc.setSelectedGrid(3.0f, 3.0f);
	ASSERT_TRUE(doc.paintAtSelection(true, false));

	doc.setSprite(liane);
	doc.setSelectedGrid(3.0f, 3.0f);
	EXPECT_FALSE(doc.paintAtSelection(true, false));
	EXPECT_EQ(0, countSprite(doc, "liane-01"));
	EXPECT_TRUE(hasSpriteAt(doc, "tile-rock-01", 3.0f, 3.0f));
}

TEST_F(MapEditorDocumentTest, testLianeInJunglePalette)
{
	MapEditorDocument doc(_mapMgr);
	doc.setTheme(ThemeTypes::JUNGLE);
	std::vector<SpriteDefPtr> palette;
	doc.collectTilePalette(palette);
	const bool found = std::any_of(palette.begin(), palette.end(), [] (const SpriteDefPtr& s) {
		return s && s->id == "liane-01";
	});
	EXPECT_TRUE(found);
}

TEST_F(MapEditorDocumentTest, testPlaceWaterfallRemovesSolidInSecondCell)
{
	MapEditorDocument doc(_mapMgr);
	const SpriteDefPtr background = requireSprite("tile-background-01");
	const SpriteDefPtr ground = requireSprite("tile-ground-01");
	const SpriteDefPtr waterfall = requireSprite("tile-waterfall-01");
	ASSERT_TRUE(!!background && !!ground && !!waterfall);

	doc.setSprite(background);
	doc.setSelectedGrid(2.0f, 1.0f);
	ASSERT_TRUE(doc.paintAtSelection(true, false));
	doc.setSelectedGrid(2.0f, 2.0f);
	ASSERT_TRUE(doc.paintAtSelection(true, false));

	doc.setSprite(ground);
	doc.setSelectedGrid(2.0f, 2.0f);
	ASSERT_TRUE(doc.paintAtSelection(true, false));
	EXPECT_TRUE(hasSpriteAt(doc, "tile-ground-01", 2.0f, 2.0f));

	doc.setSprite(waterfall);
	doc.setSelectedGrid(2.0f, 1.0f);
	ASSERT_TRUE(doc.paintAtSelection(true, false));
	EXPECT_EQ(1, countSprite(doc, "tile-waterfall-01"));
	EXPECT_TRUE(hasSpriteAt(doc, "tile-waterfall-01", 2.0f, 1.0f));
	EXPECT_FALSE(hasSpriteAt(doc, "tile-ground-01", 2.0f, 2.0f));
	EXPECT_TRUE(hasSpriteAt(doc, "tile-background-01", 2.0f, 1.0f));
	EXPECT_TRUE(hasSpriteAt(doc, "tile-background-01", 2.0f, 2.0f));
}

TEST_F(MapEditorDocumentTest, testAutoFillKeepsIntroScript)
{
	MapEditorDocument doc(_mapMgr);
	doc.setFileName("introducing-14-fish");
	doc.setMapName("In14 Fish");
	doc.setTheme(ThemeTypes::ROCK);
	doc.setSetting("seed", "1350490027");
	const char* script =
			"function onMapLoaded()\n"
			"end\n"
			"\n"
			"function intro(help)\n"
			"	help:headline(tr(\"Objectives\"))\n"
			"	help:text(tr(\"Deliver the packages without touching the fish\"))\n"
			"end\n";
	doc.setScriptLogic(script);

	doc.autoFill(ThemeTypes::ROCK);

	EXPECT_EQ("introducing-14-fish", doc.getFileName());
	EXPECT_EQ("In14 Fish", doc.getMapName());
	EXPECT_NE(std::string::npos, doc.getScriptLogic().find("function intro"))
			<< "Auto fill must not drop intro(help):\n" << doc.getScriptLogic();
	EXPECT_NE(std::string::npos, doc.getScriptLogic().find("without touching the fish"));
}

}
