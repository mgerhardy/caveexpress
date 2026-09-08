#include "tests/TestShared.h"
#include "caveexpress/client/editor/MapEditorDocument.h"
#include "caveexpress/shared/CaveExpressEntityType.h"
#include "common/MapManager.h"
#include "common/SpriteDefinition.h"
#include "common/TextureDefinition.h"
#include "common/ThemeType.h"
#include "common/Layer.h"
#include "common/vec2.h"
#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <unordered_map>
#include <vector>

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
		ASSERT_TRUE(loadEntitySizesFromLua());
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

	bool hasEntityAt (const MapEditorDocument& doc, const EntityType& type, gridCoord x, gridCoord y) const
	{
		for (const MapEditorTileItem& item : doc.getTiles()) {
			if (item.entityType != nullptr && item.entityType->name == type.name
					&& fequals(item.gridX, x) && fequals(item.gridY, y))
				return true;
		}
		return false;
	}

	void paintEmitterAt (MapEditorDocument& doc, const EntityType& type, gridCoord x, gridCoord y)
	{
		doc.setEmitterEntity(type);
		doc.setCursorGrid(x, y);
		doc.setSelectedGrid(std::floor(x), std::floor(y));
		ASSERT_TRUE(doc.paintAtSelection(true, false));
	}

	// Mirrors UIMapEditorWindow::renderMapIntoCanvas + mapEditorAddSprite.
	// ImGui and AbstractGLFrontend both merge consecutive quads with the same
	// texture id, so batchBreaks is the number of draw commands those backends
	// would emit for the sprite images.
	struct EditorRenderStats {
		int tilesTotal = 0;
		int tilesWalked = 0;
		int tilesVisible = 0;
		int images = 0;
		int batchBreaks = 0;
		int uniqueAtlases = 0;
		int layersPerVisibleTile = 0;
	};

	std::string atlasForFrame (const std::string& frameName) const
	{
		if (!_textures->exists(frameName))
			return frameName;
		return _textures->getTextureDef(frameName).textureName;
	}

	void emitImage (EditorRenderStats& stats, const std::string& atlas, std::string& lastAtlas,
			bool& haveAtlas, std::unordered_map<std::string, int>& atlasHits) const
	{
		++stats.images;
		++atlasHits[atlas];
		if (!haveAtlas || atlas != lastAtlas) {
			++stats.batchBreaks;
			lastAtlas = atlas;
			haveAtlas = true;
		}
	}

	bool tileOnScreen (const MapEditorDocument& doc, const MapEditorTileItem& item,
			float startGX, float startGY, int visibleW, int visibleH) const
	{
		if (!doc.isLayerActive(item.layer))
			return false;
		if (item.gridX < startGX - 2 || item.gridY < startGY - 2)
			return false;
		if (item.gridX >= startGX + visibleW || item.gridY >= startGY + visibleH)
			return false;
		return true;
	}

	void simulateEditorDraw (const MapEditorDocument& doc, float startGX, float startGY,
			int visibleW, int visibleH, bool tileMajor, EditorRenderStats& stats) const
	{
		stats.tilesTotal = static_cast<int>(doc.getTiles().size());
		std::string lastAtlas;
		bool haveAtlas = false;
		std::unordered_map<std::string, int> atlasHits;

		if (tileMajor) {
			for (const MapEditorTileItem& item : doc.getTiles()) {
				++stats.tilesWalked;
				if (!tileOnScreen(doc, item, startGX, startGY, visibleW, visibleH))
					continue;
				++stats.tilesVisible;
				if (!item.def)
					continue;
				for (Layer layer = LAYER_BACK; layer < MAX_LAYERS; ++layer) {
					if (item.def->textures[layer].empty())
						continue;
					++stats.layersPerVisibleTile;
					emitImage(stats, atlasForFrame(item.def->textures[layer].front().name), lastAtlas,
							haveAtlas, atlasHits);
				}
			}
		} else {
			for (Layer layer = LAYER_BACK; layer < MAX_LAYERS; ++layer) {
				for (const MapEditorTileItem& item : doc.getTiles()) {
					++stats.tilesWalked;
					if (!tileOnScreen(doc, item, startGX, startGY, visibleW, visibleH))
						continue;
					if (layer == LAYER_BACK)
						++stats.tilesVisible;
					if (!item.def || item.def->textures[layer].empty())
						continue;
					emitImage(stats, atlasForFrame(item.def->textures[layer].front().name), lastAtlas,
							haveAtlas, atlasHits);
				}
			}
			for (const MapEditorTileItem& item : doc.getTiles()) {
				if (!item.def || !tileOnScreen(doc, item, startGX, startGY, visibleW, visibleH))
					continue;
				for (Layer layer = LAYER_BACK; layer < MAX_LAYERS; ++layer) {
					if (!item.def->textures[layer].empty())
						++stats.layersPerVisibleTile;
				}
			}
		}
		stats.uniqueAtlases = static_cast<int>(atlasHits.size());
	}

	void simulateAtlasGrouped (const MapEditorDocument& doc, float startGX, float startGY,
			int visibleW, int visibleH, EditorRenderStats& stats) const
	{
		stats.tilesTotal = static_cast<int>(doc.getTiles().size());
		struct Quad {
			int mapLayer;
			int spriteLayer;
			std::string atlas;
		};
		std::vector<Quad> images;
		images.reserve(doc.getTiles().size() * 2);
		for (const MapEditorTileItem& item : doc.getTiles()) {
			++stats.tilesWalked;
			if (!tileOnScreen(doc, item, startGX, startGY, visibleW, visibleH))
				continue;
			++stats.tilesVisible;
			if (!item.def)
				continue;
			for (Layer layer = LAYER_BACK; layer < MAX_LAYERS; ++layer) {
				if (item.def->textures[layer].empty())
					continue;
				++stats.layersPerVisibleTile;
				Quad q;
				q.mapLayer = item.layer;
				q.spriteLayer = layer;
				q.atlas = atlasForFrame(item.def->textures[layer].front().name);
				images.push_back(q);
			}
		}
		std::stable_sort(images.begin(), images.end(), [] (const Quad& a, const Quad& b) {
			if (a.mapLayer != b.mapLayer)
				return a.mapLayer < b.mapLayer;
			if (a.spriteLayer != b.spriteLayer)
				return a.spriteLayer < b.spriteLayer;
			return a.atlas < b.atlas;
		});
		std::string lastAtlas;
		bool haveAtlas = false;
		std::unordered_map<std::string, int> atlasHits;
		for (const Quad& q : images)
			emitImage(stats, q.atlas, lastAtlas, haveAtlas, atlasHits);
		stats.uniqueAtlases = static_cast<int>(atlasHits.size());
	}

	void printStats (const char* label, const EditorRenderStats& s) const
	{
		const float layersAvg = s.tilesVisible > 0
				? static_cast<float>(s.layersPerVisibleTile) / static_cast<float>(s.tilesVisible) : 0.0f;
		std::printf("  %-36s tiles=%5d walked=%5d vis=%5d images=%5d cmds=%5d atlases=%2d layers/tile=%.2f\n",
				label, s.tilesTotal, s.tilesWalked, s.tilesVisible, s.images, s.batchBreaks,
				s.uniqueAtlases, layersAvg);
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

TEST_F(MapEditorDocumentTest, testReplaceOverlappingEmitters)
{
	MapEditorDocument doc(_mapMgr);
	paintEmitterAt(doc, EntityTypes::APPLE, 2.0f, 3.0f);
	paintEmitterAt(doc, EntityTypes::APPLE, 2.0f, 3.0f);
	EXPECT_EQ(1, doc.countEntitiesOfType(EntityTypes::APPLE));
	EXPECT_TRUE(hasEntityAt(doc, EntityTypes::APPLE, 2.0f, 3.0f));
}

TEST_F(MapEditorDocumentTest, testAllowNonOverlappingEmittersInSameCell)
{
	MapEditorDocument doc(_mapMgr);
	paintEmitterAt(doc, EntityTypes::APPLE, 2.0f, 3.0f);
	paintEmitterAt(doc, EntityTypes::APPLE, 2.5f, 3.0f);
	EXPECT_EQ(2, doc.countEntitiesOfType(EntityTypes::APPLE));
	EXPECT_TRUE(hasEntityAt(doc, EntityTypes::APPLE, 2.0f, 3.0f));
	EXPECT_TRUE(hasEntityAt(doc, EntityTypes::APPLE, 2.5f, 3.0f));
}

TEST_F(MapEditorDocumentTest, testPlaceEmitterAtFractionalCursor)
{
	MapEditorDocument doc(_mapMgr);
	paintEmitterAt(doc, EntityTypes::APPLE, 2.37f, 3.12f);
	EXPECT_EQ(1, doc.countEntitiesOfType(EntityTypes::APPLE));
	EXPECT_TRUE(hasEntityAt(doc, EntityTypes::APPLE, 2.4f, 3.1f));
}

TEST_F(MapEditorDocumentTest, testPlaceEmitterOnSolidLiftsAndKeepsGround)
{
	MapEditorDocument doc(_mapMgr);
	const SpriteDefPtr ground = requireSprite("tile-ground-01");
	ASSERT_TRUE(!!ground);

	doc.setSprite(ground);
	doc.setSelectedGrid(2.0f, 4.0f);
	ASSERT_TRUE(doc.paintAtSelection(true, false));

	paintEmitterAt(doc, EntityTypes::APPLE, 2.0f, 4.0f);
	EXPECT_TRUE(hasSpriteAt(doc, "tile-ground-01", 2.0f, 4.0f));
	EXPECT_EQ(1, doc.countEntitiesOfType(EntityTypes::APPLE));
	EXPECT_TRUE(hasEntityAt(doc, EntityTypes::APPLE, 2.0f, 3.0f));

	paintEmitterAt(doc, EntityTypes::NPC_WALKING, 2.0f, 4.0f);
	EXPECT_TRUE(hasSpriteAt(doc, "tile-ground-01", 2.0f, 4.0f));
	EXPECT_EQ(1, doc.countEntitiesOfType(EntityTypes::NPC_WALKING));
	EXPECT_TRUE(hasEntityAt(doc, EntityTypes::NPC_WALKING, 2.0f, 3.0f));
}

TEST_F(MapEditorDocumentTest, testSolidBuriesOverlappingEmitters)
{
	MapEditorDocument doc(_mapMgr);
	const SpriteDefPtr rock = requireSprite("tile-rock-01");
	const SpriteDefPtr ground = requireSprite("tile-ground-01");
	ASSERT_TRUE(!!rock && !!ground);

	doc.setSprite(ground);
	doc.setSelectedGrid(2.0f, 4.0f);
	ASSERT_TRUE(doc.paintAtSelection(true, false));
	doc.setSelectedGrid(3.0f, 4.0f);
	ASSERT_TRUE(doc.paintAtSelection(true, false));

	paintEmitterAt(doc, EntityTypes::TREE, 2.0f, 2.0f);
	EXPECT_EQ(1, doc.countEntitiesOfType(EntityTypes::TREE));

	doc.setSprite(rock);
	doc.setSelectedGrid(2.0f, 3.0f);
	ASSERT_TRUE(doc.paintAtSelection(true, false));
	EXPECT_EQ(0, doc.countEntitiesOfType(EntityTypes::TREE));
	EXPECT_TRUE(hasSpriteAt(doc, "tile-rock-01", 2.0f, 3.0f));
	EXPECT_TRUE(hasSpriteAt(doc, "tile-ground-01", 2.0f, 4.0f));
}

TEST_F(MapEditorDocumentTest, testBigSolidOverlapsTreeBoundingBox)
{
	ASSERT_FLOAT_EQ(2.0f, EntityTypes::TREE.width);
	ASSERT_FLOAT_EQ(2.0f, EntityTypes::TREE.height);
	MapEditorDocument doc(_mapMgr);
	const SpriteDefPtr rock = requireSprite("tile-rock-big-01");
	ASSERT_TRUE(!!rock);
	EXPECT_FLOAT_EQ(2.0f, rock->width);
	EXPECT_FLOAT_EQ(2.0f, rock->height);

	paintEmitterAt(doc, EntityTypes::TREE, 2.0f, 2.0f);
	EXPECT_EQ(1, doc.countEntitiesOfType(EntityTypes::TREE));

	// 2x2 rock one cell right and one cell down: foliage sits above the rock,
	// trunk overlaps the left column of the rock (the reported editor case).
	doc.setSprite(rock);
	doc.setSelectedGrid(3.0f, 3.0f);
	ASSERT_TRUE(doc.paintAtSelection(true, false));
	EXPECT_EQ(0, doc.countEntitiesOfType(EntityTypes::TREE))
			<< "2x2 solid must bury a 2x2 tree whose AABB overlaps it";
	EXPECT_TRUE(hasSpriteAt(doc, "tile-rock-big-01", 3.0f, 3.0f));
}

TEST_F(MapEditorDocumentTest, testRepaintSolidBuriesOverlappingTree)
{
	MapEditorDocument doc(_mapMgr);
	const SpriteDefPtr rock = requireSprite("tile-rock-big-01");
	ASSERT_TRUE(!!rock);

	doc.setSprite(rock);
	doc.setSelectedGrid(3.0f, 3.0f);
	ASSERT_TRUE(doc.paintAtSelection(true, false));

	// Emitters lift off solids on click-place; move the tree back so it
	// overlaps the existing 2x2 rock (same state as a loaded map).
	doc.setEditMode(IMapEditorDocument::EditMode::Entities);
	paintEmitterAt(doc, EntityTypes::TREE, 2.0f, 2.0f);
	ASSERT_EQ(1, doc.countEntitiesOfType(EntityTypes::TREE));
	doc.setHighlightFromSelection();
	ASSERT_NE(nullptr, doc.getHighlightItem());
	doc.setHighlightPosition(2.0f, 2.0f);
	doc.setEditMode(IMapEditorDocument::EditMode::Tiles);
	EXPECT_EQ(1, doc.countEntitiesOfType(EntityTypes::TREE));

	doc.setSprite(rock);
	doc.setSelectedGrid(3.0f, 3.0f);
	ASSERT_TRUE(doc.paintAtSelection(true, false));
	EXPECT_EQ(0, doc.countEntitiesOfType(EntityTypes::TREE))
			<< "Re-painting an existing solid must still bury overlapping emitters";
}

TEST_F(MapEditorDocumentTest, testSolidUnderEmitterFeetDoesNotRemoveIt)
{
	MapEditorDocument doc(_mapMgr);
	const SpriteDefPtr rock = requireSprite("tile-rock-01");
	ASSERT_TRUE(!!rock);

	paintEmitterAt(doc, EntityTypes::TREE, 2.0f, 2.0f);
	doc.setSprite(rock);
	doc.setSelectedGrid(2.0f, 4.0f);
	ASSERT_TRUE(doc.paintAtSelection(true, false));
	EXPECT_EQ(1, doc.countEntitiesOfType(EntityTypes::TREE));
	EXPECT_TRUE(hasSpriteAt(doc, "tile-rock-01", 2.0f, 4.0f));
}

TEST_F(MapEditorDocumentTest, testEditorRenderCostRacesVsSmall)
{
	MapEditorDocument races(_mapMgr);
	MapEditorDocument intro(_mapMgr);
	ASSERT_TRUE(races.load("races-01"));
	ASSERT_TRUE(intro.load("introducing-01-package"));
	ASSERT_GT(races.getTiles().size(), 2000u);
	ASSERT_LT(intro.getTiles().size(), 400u);

	struct View {
		const char* name;
		float startGX;
		float startGY;
		int visibleW;
		int visibleH;
	};
	// Fit-on-open for a 56x61 map in a ~1280x720 canvas shows almost everything.
	// Zoomed-in is a typical close view (~20x12 tiles plus the editor +3 margin).
	const View views[] = {
		{ "fit", 0.0f, 0.0f, races.getMapWidth() + 3, races.getMapHeight() + 3 },
		{ "zoomed-in", 20.0f, 20.0f, 23, 15 },
	};

	std::printf("\nEditor canvas cost (ImGui AddImage / GL3+SDL merge same tex id)\n");
	std::printf("races-01 %dx%d  introducing-01-package %dx%d\n",
			races.getMapWidth(), races.getMapHeight(), intro.getMapWidth(), intro.getMapHeight());

	EditorRenderStats racesFitEditor, racesZoomEditor, racesFitLayer, racesZoomLayer, racesFitAtlas;
	EditorRenderStats introFitEditor, introZoomEditor, introFitLayer, introFitAtlas;

	for (const View& view : views) {
		EditorRenderStats editor, layerMajor, atlasGrouped;
		simulateEditorDraw(races, view.startGX, view.startGY, view.visibleW, view.visibleH, true, editor);
		simulateEditorDraw(races, view.startGX, view.startGY, view.visibleW, view.visibleH, false, layerMajor);
		simulateAtlasGrouped(races, view.startGX, view.startGY, view.visibleW, view.visibleH, atlasGrouped);
		std::printf("races-01 %s\n", view.name);
		printStats("list order (old)", editor);
		printStats("game layer-major", layerMajor);
		printStats("editor grouped (current)", atlasGrouped);
		if (std::string(view.name) == "fit") {
			racesFitEditor = editor;
			racesFitLayer = layerMajor;
			racesFitAtlas = atlasGrouped;
		} else {
			racesZoomEditor = editor;
			racesZoomLayer = layerMajor;
		}
	}

	simulateEditorDraw(intro, 0.0f, 0.0f, intro.getMapWidth() + 3, intro.getMapHeight() + 3, true, introFitEditor);
	simulateEditorDraw(intro, 0.0f, 0.0f, intro.getMapWidth() + 3, intro.getMapHeight() + 3, false, introFitLayer);
	simulateAtlasGrouped(intro, 0.0f, 0.0f, intro.getMapWidth() + 3, intro.getMapHeight() + 3, introFitAtlas);
	simulateEditorDraw(intro, 4.0f, 3.0f, 23, 15, true, introZoomEditor);
	std::printf("introducing-01-package fit\n");
	printStats("list order (old)", introFitEditor);
	printStats("game layer-major", introFitLayer);
	printStats("editor grouped (current)", introFitAtlas);
	std::printf("introducing-01-package zoomed-in\n");
	printStats("list order (old)", introZoomEditor);

	const int frames = 200;
	auto timeDraw = [&] (const MapEditorDocument& doc, float gx, float gy, int vw, int vh, bool tileMajor) {
		using Clock = std::chrono::steady_clock;
		const Clock::time_point t0 = Clock::now();
		volatile int sink = 0;
		for (int i = 0; i < frames; ++i) {
			EditorRenderStats s;
			simulateEditorDraw(doc, gx, gy, vw, vh, tileMajor, s);
			sink += s.images + s.batchBreaks + s.tilesWalked;
		}
		const double ms = std::chrono::duration<double, std::milli>(Clock::now() - t0).count();
		(void)sink;
		return ms / frames;
	};

	const double racesFitMs = timeDraw(races, 0.0f, 0.0f, races.getMapWidth() + 3, races.getMapHeight() + 3, true);
	const double racesZoomMs = timeDraw(races, 20.0f, 20.0f, 23, 15, true);
	const double racesFitLayerMs = timeDraw(races, 0.0f, 0.0f, races.getMapWidth() + 3, races.getMapHeight() + 3, false);
	const double introFitMs = timeDraw(intro, 0.0f, 0.0f, intro.getMapWidth() + 3, intro.getMapHeight() + 3, true);

	std::printf("ms/frame (%d iters)  races fit=%.4f  races zoomed-in=%.4f  races layer-major=%.4f  intro fit=%.4f\n",
			frames, racesFitMs, racesZoomMs, racesFitLayerMs, introFitMs);
	std::printf("grid lines (full map, now view-culled in the editor): races=%d  intro=%d\n",
			races.getMapWidth() + races.getMapHeight() + 2,
			intro.getMapWidth() + intro.getMapHeight() + 2);

	EXPECT_GT(racesFitEditor.tilesTotal, introFitEditor.tilesTotal * 8);
	EXPECT_LT(racesZoomEditor.tilesVisible, racesFitEditor.tilesVisible / 2)
			<< "zoom-in must drop visible tiles; if editor cost stays flat the walk or cmds dominate";
	EXPECT_EQ(racesFitEditor.batchBreaks, racesFitLayer.batchBreaks)
			<< "races-01 tiles are single-layer; layer-major order should not change cmd count";
	EXPECT_LE(racesFitAtlas.batchBreaks, racesFitAtlas.uniqueAtlases * LAYER_MAX)
			<< "grouping by map layer + atlas should stay near one cmd per atlas per document layer";
	EXPECT_GT(racesFitEditor.batchBreaks, racesFitAtlas.batchBreaks * 10)
			<< "list order fragments atlases; grouping must cut draw cmds sharply";
	EXPECT_GT(racesFitEditor.images, introFitEditor.images * 8);
	(void)racesZoomLayer;
}

TEST_F(MapEditorDocumentTest, testEditorRenderPerfRaces01)
{
	if (std::getenv("CAVEEXPRESS_EDITOR_PERF") == nullptr)
		GTEST_SKIP() << "set CAVEEXPRESS_EDITOR_PERF=1 to run the long races-01 loop";
	MapEditorDocument doc(_mapMgr);
	ASSERT_TRUE(doc.load("races-01"));
	const int frames = 8000;
	const int vw = doc.getMapWidth() + 3;
	const int vh = doc.getMapHeight() + 3;
	volatile int sink = 0;
	for (int i = 0; i < frames; ++i) {
		EditorRenderStats s;
		simulateAtlasGrouped(doc, 0.0f, 0.0f, vw, vh, s);
		sink += s.images + s.batchBreaks;
	}
	EXPECT_GT(sink, 0);
}

TEST_F(MapEditorDocumentTest, testEditorRenderPerfIntroducing01)
{
	if (std::getenv("CAVEEXPRESS_EDITOR_PERF") == nullptr)
		GTEST_SKIP() << "set CAVEEXPRESS_EDITOR_PERF=1 to run the long introducing-01 loop";
	MapEditorDocument doc(_mapMgr);
	ASSERT_TRUE(doc.load("introducing-01-package"));
	const int frames = 8000;
	const int vw = doc.getMapWidth() + 3;
	const int vh = doc.getMapHeight() + 3;
	volatile int sink = 0;
	for (int i = 0; i < frames; ++i) {
		EditorRenderStats s;
		simulateAtlasGrouped(doc, 0.0f, 0.0f, vw, vh, s);
		sink += s.images + s.batchBreaks;
	}
	EXPECT_GT(sink, 0);
}

}
