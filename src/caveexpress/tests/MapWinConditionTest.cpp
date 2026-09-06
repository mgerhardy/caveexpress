#include "tests/TestShared.h"
#include "caveexpress/shared/MapValidator.h"
#include "caveexpress/shared/CaveExpressMapContext.h"
#include "caveexpress/shared/CaveExpressEntityType.h"
#include "common/MapManager.h"
#include "common/MapSettings.h"
#include "common/SpriteDefinition.h"
#include "common/String.h"
#include "common/TextureDefinition.h"
#include <sstream>

namespace caveexpress {

class MapWinConditionTest: public AbstractTest {
protected:
	TextureDefinition* _textures = nullptr;

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

	void addCaveWithPlatform (std::vector<MapTileDefinition>& tiles, std::vector<CaveTileDefinition>& caves,
			int x, int y, const EntityType& npc = EntityType::NONE) const
	{
		const SpriteDefPtr caveDef = requireSprite("tile-cave-01");
		const SpriteDefPtr ground = requireSprite("tile-ground-01");
		ASSERT_TRUE(!!caveDef);
		ASSERT_TRUE(!!ground);
		caves.emplace_back(x, y, caveDef, npc, 1000);
		tiles.emplace_back(x, y + 1, ground, 0);
	}

	static std::string joinIssues (const MapWinCondition& win)
	{
		std::ostringstream out;
		for (size_t i = 0; i < win.issues.size(); ++i) {
			if (i > 0)
				out << "; ";
			out << win.issues[i];
		}
		return out.str();
	}

	/** Lua stores gates/plates separately from addTile; they still occupy cells. */
	static std::vector<MapTileDefinition> occupancyTiles (const CaveExpressMapContext& ctx)
	{
		std::vector<MapTileDefinition> tiles = ctx.getMapTileDefinitions();
		for (const GateDefinition& gate : ctx.getGateDefinitions())
			tiles.emplace_back(gate.x, gate.y, gate.spriteDef, 0);
		for (const PressurePlateDefinition& plate : ctx.getPressurePlateDefinitions())
			tiles.emplace_back(plate.x, plate.y, plate.spriteDef, 0);
		return tiles;
	}
};

TEST_F(MapWinConditionTest, testPackageGoalWithoutTarget)
{
	IMap::SettingsMap settings;
	settings[msn::PACKAGE_TRANSFER_COUNT] = "1";
	std::vector<MapTileDefinition> tiles;
	std::vector<CaveTileDefinition> caves;
	std::vector<EmitterDefinition> emitters;
	addCaveWithPlatform(tiles, caves, 1, 2);

	const MapWinCondition win = MapValidator::checkWinConditions(settings, tiles, caves, emitters);
	EXPECT_FALSE(win.winnable);
	EXPECT_FALSE(win.issues.empty());
}

TEST_F(MapWinConditionTest, testPackageGoalWithoutSource)
{
	IMap::SettingsMap settings;
	settings[msn::PACKAGE_TRANSFER_COUNT] = "4";
	std::vector<MapTileDefinition> tiles;
	std::vector<CaveTileDefinition> caves;
	std::vector<EmitterDefinition> emitters;
	const SpriteDefPtr target = requireSprite("tile-packagetarget-rock-01-idle");
	ASSERT_TRUE(!!target);
	tiles.emplace_back(2, 3, target, 0);

	const MapWinCondition win = MapValidator::checkWinConditions(settings, tiles, caves, emitters);
	EXPECT_FALSE(win.winnable) << joinIssues(win);
}

TEST_F(MapWinConditionTest, testPackageQuotaMetByRespawningCaves)
{
	IMap::SettingsMap settings;
	settings[msn::PACKAGE_TRANSFER_COUNT] = "4";
	std::vector<MapTileDefinition> tiles;
	std::vector<CaveTileDefinition> caves;
	std::vector<EmitterDefinition> emitters;
	const SpriteDefPtr target = requireSprite("tile-packagetarget-rock-01-idle");
	ASSERT_TRUE(!!target);
	tiles.emplace_back(2, 3, target, 0);
	addCaveWithPlatform(tiles, caves, 1, 2);
	addCaveWithPlatform(tiles, caves, 5, 7);
	addCaveWithPlatform(tiles, caves, 14, 8);

	const MapWinCondition win = MapValidator::checkWinConditions(settings, tiles, caves, emitters);
	EXPECT_TRUE(win.winnable) << joinIssues(win);
}

TEST_F(MapWinConditionTest, testNpcTransferNeedsTwoCaves)
{
	IMap::SettingsMap settings;
	settings[msn::NPC_TRANSFER_COUNT] = "1";
	std::vector<MapTileDefinition> tiles;
	std::vector<CaveTileDefinition> caves;
	std::vector<EmitterDefinition> emitters;
	addCaveWithPlatform(tiles, caves, 1, 2, EntityTypes::NPC_FRIENDLY_MAN);

	EXPECT_FALSE(MapValidator::checkWinConditions(settings, tiles, caves, emitters).winnable);

	addCaveWithPlatform(tiles, caves, 6, 2);
	EXPECT_TRUE(MapValidator::checkWinConditions(settings, tiles, caves, emitters).winnable);
}

TEST_F(MapWinConditionTest, testCaveMustReplaceExistingTile)
{
	IMap::SettingsMap settings;
	settings[msn::NPC_TRANSFER_COUNT] = "1";
	std::vector<MapTileDefinition> tiles;
	std::vector<CaveTileDefinition> caves;
	std::vector<EmitterDefinition> emitters;
	addCaveWithPlatform(tiles, caves, 1, 2, EntityTypes::NPC_FRIENDLY_MAN);
	addCaveWithPlatform(tiles, caves, 6, 2);
	EXPECT_TRUE(MapValidator::checkWinConditions(settings, tiles, caves, emitters).winnable);

	const SpriteDefPtr background = requireSprite("tile-background-04");
	ASSERT_TRUE(!!background);
	tiles.emplace_back(1, 2, background, 0);
	const MapWinCondition overlap = MapValidator::checkWinConditions(settings, tiles, caves, emitters);
	EXPECT_FALSE(overlap.winnable) << joinIssues(overlap);
}

TEST_F(MapWinConditionTest, testCaveAllowsBridgeOverlay)
{
	IMap::SettingsMap settings;
	settings[msn::NPC_TRANSFER_COUNT] = "1";
	std::vector<MapTileDefinition> tiles;
	std::vector<CaveTileDefinition> caves;
	std::vector<EmitterDefinition> emitters;
	addCaveWithPlatform(tiles, caves, 1, 2, EntityTypes::NPC_FRIENDLY_MAN);
	addCaveWithPlatform(tiles, caves, 6, 2);
	const SpriteDefPtr plank = requireSprite("bridge-plank-01");
	ASSERT_TRUE(!!plank);
	tiles.emplace_back(1, 2, plank, 0);
	const MapWinCondition win = MapValidator::checkWinConditions(settings, tiles, caves, emitters);
	EXPECT_TRUE(win.winnable) << joinIssues(win);
}

TEST_F(MapWinConditionTest, testCaveNeedsGroundBelow)
{
	IMap::SettingsMap settings;
	settings[msn::NPC_TRANSFER_COUNT] = "1";
	std::vector<MapTileDefinition> tiles;
	std::vector<CaveTileDefinition> caves;
	std::vector<EmitterDefinition> emitters;
	const SpriteDefPtr caveDef = requireSprite("tile-cave-01");
	ASSERT_TRUE(!!caveDef);
	caves.emplace_back(1, 2, caveDef, EntityTypes::NPC_FRIENDLY_MAN, 1000);
	caves.emplace_back(6, 2, caveDef, EntityType::NONE, 1000);
	const MapWinCondition win = MapValidator::checkWinConditions(settings, tiles, caves, emitters);
	EXPECT_FALSE(win.winnable) << joinIssues(win);
}

TEST_F(MapWinConditionTest, testFiniteEmittersCannotMeetQuotaWithoutCaves)
{
	IMap::SettingsMap settings;
	settings[msn::PACKAGE_TRANSFER_COUNT] = "4";
	std::vector<MapTileDefinition> tiles;
	std::vector<CaveTileDefinition> caves;
	std::vector<EmitterDefinition> emitters;
	const SpriteDefPtr target = requireSprite("tile-packagetarget-rock-01-idle");
	ASSERT_TRUE(!!target);
	tiles.emplace_back(2, 3, target, 0);
	emitters.emplace_back(1, 1, EntityTypes::PACKAGE_ROCK, 2, 0, "");

	EXPECT_FALSE(MapValidator::checkWinConditions(settings, tiles, caves, emitters).winnable);

	emitters.emplace_back(3, 1, EntityTypes::PACKAGE_ROCK, 0, 0, "");
	EXPECT_TRUE(MapValidator::checkWinConditions(settings, tiles, caves, emitters).winnable);
}

TEST_F(MapWinConditionTest, testNothingToDo)
{
	IMap::SettingsMap settings;
	settings[msn::PACKAGE_TRANSFER_COUNT] = "0";
	settings[msn::NPC_TRANSFER_COUNT] = "0";
	const MapWinCondition win = MapValidator::checkWinConditions(settings, {}, {}, {});
	EXPECT_FALSE(win.winnable);
}

TEST_F(MapWinConditionTest, testCutsceneSkipped)
{
	IMap::SettingsMap settings;
	settings[msn::CUTSCENE] = "true";
	settings[msn::PACKAGE_TRANSFER_COUNT] = "0";
	settings[msn::NPC_TRANSFER_COUNT] = "0";
	const MapWinCondition win = MapValidator::checkWinConditions(settings, {}, {}, {});
	EXPECT_TRUE(win.winnable);
}

TEST_F(MapWinConditionTest, testEveryCellMustBeFilled)
{
	IMap::SettingsMap settings;
	settings[msn::WIDTH] = "3";
	settings[msn::HEIGHT] = "2";
	settings[msn::PACKAGE_TRANSFER_COUNT] = "1";
	std::vector<MapTileDefinition> tiles;
	std::vector<CaveTileDefinition> caves;
	std::vector<EmitterDefinition> emitters;
	const SpriteDefPtr bg = requireSprite("tile-background-01");
	const SpriteDefPtr ground = requireSprite("tile-ground-01");
	const SpriteDefPtr target = requireSprite("tile-packagetarget-rock-01-idle");
	const SpriteDefPtr caveDef = requireSprite("tile-cave-01");
	ASSERT_TRUE(!!bg);
	ASSERT_TRUE(!!ground);
	ASSERT_TRUE(!!target);
	ASSERT_TRUE(!!caveDef);
	tiles.emplace_back(0, 0, bg, 0);
	caves.emplace_back(1, 0, caveDef, EntityType::NONE, 1000);
	tiles.emplace_back(2, 0, bg, 0);
	tiles.emplace_back(0, 1, ground, 0);
	tiles.emplace_back(1, 1, target, 0);
	tiles.emplace_back(2, 1, ground, 0);
	EXPECT_TRUE(MapValidator::checkWinConditions(settings, tiles, caves, emitters).winnable);

	tiles.pop_back();
	const MapWinCondition hole = MapValidator::checkWinConditions(settings, tiles, caves, emitters);
	EXPECT_FALSE(hole.winnable) << joinIssues(hole);
}

TEST_F(MapWinConditionTest, testBridgeAloneDoesNotFillCell)
{
	IMap::SettingsMap settings;
	settings[msn::WIDTH] = "3";
	settings[msn::HEIGHT] = "2";
	settings[msn::PACKAGE_TRANSFER_COUNT] = "1";
	std::vector<MapTileDefinition> tiles;
	std::vector<CaveTileDefinition> caves;
	std::vector<EmitterDefinition> emitters;
	const SpriteDefPtr bg = requireSprite("tile-background-01");
	const SpriteDefPtr ground = requireSprite("tile-ground-01");
	const SpriteDefPtr target = requireSprite("tile-packagetarget-rock-01-idle");
	const SpriteDefPtr caveDef = requireSprite("tile-cave-01");
	const SpriteDefPtr plank = requireSprite("bridge-plank-01");
	ASSERT_TRUE(!!bg);
	ASSERT_TRUE(!!ground);
	ASSERT_TRUE(!!target);
	ASSERT_TRUE(!!caveDef);
	ASSERT_TRUE(!!plank);

	auto fillHostTiles = [&] (bool hostAtBridgeCell) {
		tiles.clear();
		caves.clear();
		tiles.emplace_back(0, 0, bg, 0);
		caves.emplace_back(1, 0, caveDef, EntityType::NONE, 1000);
		if (hostAtBridgeCell)
			tiles.emplace_back(2, 0, bg, 0);
		tiles.emplace_back(2, 0, plank, 0);
		tiles.emplace_back(0, 1, ground, 0);
		tiles.emplace_back(1, 1, target, 0);
		tiles.emplace_back(2, 1, ground, 0);
	};

	fillHostTiles(false);
	const MapWinCondition hole = MapValidator::checkWinConditions(settings, tiles, caves, emitters);
	EXPECT_FALSE(hole.winnable) << joinIssues(hole);

	fillHostTiles(true);
	const MapWinCondition filled = MapValidator::checkWinConditions(settings, tiles, caves, emitters);
	EXPECT_TRUE(filled.winnable) << joinIssues(filled);
}

TEST_F(MapWinConditionTest, testRock01FourPackagesWithThreeCaves)
{
	CaveExpressMapContext ctx("rock-01");
	ASSERT_TRUE(ctx.load(true));
	ASSERT_EQ(3u, ctx.getCaveTileDefinitions().size());
	const auto pkg = ctx.getSettings().find(msn::PACKAGE_TRANSFER_COUNT);
	ASSERT_TRUE(pkg != ctx.getSettings().end());
	EXPECT_EQ("4", pkg->second);
	const MapWinCondition win = MapValidator::checkWinConditions(ctx.getSettings(),
			occupancyTiles(ctx), ctx.getCaveTileDefinitions(), ctx.getEmitterDefinitions());
	EXPECT_TRUE(win.winnable) << joinIssues(win);
}

TEST_F(MapWinConditionTest, testAllMapsWinConditions)
{
	LUAMapManager mgr;
	mgr.loadMaps();
	ASSERT_FALSE(mgr.getMaps().empty());

	std::ostringstream failed;
	int checked = 0;
	for (const auto& entry : mgr.getMaps()) {
		const std::string& id = entry.first;
		if (string::startsWith(id, "test") || string::startsWith(id, "empty"))
			continue;
		CaveExpressMapContext ctx(id);
		if (!ctx.load(true)) {
			failed << id << ": failed to load\n";
			continue;
		}
		++checked;
		const MapWinCondition win = MapValidator::checkWinConditions(ctx.getSettings(),
				occupancyTiles(ctx), ctx.getCaveTileDefinitions(), ctx.getEmitterDefinitions());
		if (win.winnable)
			continue;
		failed << id;
		for (const std::string& issue : win.issues)
			failed << "\n  " << issue;
		failed << '\n';
	}
	EXPECT_GT(checked, 0);
	EXPECT_TRUE(failed.str().empty()) << "maps with unwinnable Lua success rules:\n" << failed.str();
}

}
