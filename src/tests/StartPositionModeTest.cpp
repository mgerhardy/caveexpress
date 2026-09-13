#include "tests/TestShared.h"
#include "common/IMap.h"
#include "common/StartPositionMode.h"
#include <utility>

TEST(StartPositionModeTest, parseAliasesDefaultToBoth)
{
	EXPECT_EQ(StartPositionModes::BOTH, StartPositionModes::fromString(""));
	EXPECT_EQ(StartPositionModes::BOTH, StartPositionModes::fromString("both"));
	EXPECT_EQ(StartPositionModes::BOTH, StartPositionModes::fromString("unknown"));
	EXPECT_EQ(StartPositionModes::SINGLEPLAYER, StartPositionModes::fromString("SP"));
	EXPECT_EQ(StartPositionModes::SINGLEPLAYER, StartPositionModes::fromString("single"));
	EXPECT_EQ(StartPositionModes::MULTIPLAYER, StartPositionModes::fromString("mp"));
	EXPECT_EQ(StartPositionModes::MULTIPLAYER, StartPositionModes::fromString("Multiplayer"));
	EXPECT_STREQ("both", StartPositionModes::toString(StartPositionModes::BOTH));
	EXPECT_STREQ("singleplayer", StartPositionModes::toString(StartPositionModes::SINGLEPLAYER));
	EXPECT_STREQ("multiplayer", StartPositionModes::toString(StartPositionModes::MULTIPLAYER));
}

TEST(StartPositionModeTest, luaCountsIgnoreSingleplayerOnly)
{
	const std::string lua =
		"map:addStartPosition(\"1\", \"2\")\n"
		"map:addStartPosition(\"3\", \"4\", \"multiplayer\")\n"
		"map:addStartPosition(\"5\", \"6\", \"singleplayer\")\n";
	EXPECT_EQ(2, StartPositionModes::countMultiplayerStartsFromLua(lua));
	EXPECT_EQ(0, StartPositionModes::countMultiplayerStartsFromLua("function initMap() end"));
}

TEST(StartPositionModeTest, sokobanCountsUseStartModes)
{
	const std::string unmarked =
		";plain\n"
		"#####\n"
		"#@$.#\n"
		"#####\n";
	EXPECT_EQ(1, StartPositionModes::countMultiplayerStartsFromSokoban(unmarked));

	const std::string mixed =
		";mixed\n"
		"StartModes: both,multiplayer\n"
		"######\n"
		"#@$.@#\n"
		"######\n";
	EXPECT_EQ(2, StartPositionModes::countMultiplayerStartsFromSokoban(mixed));

	const std::string spOnlySecond =
		";sp\n"
		"StartModes: both,singleplayer\n"
		"######\n"
		"#@$.@#\n"
		"######\n";
	EXPECT_EQ(1, StartPositionModes::countMultiplayerStartsFromSokoban(spOnlySecond));
}

TEST(StartPositionModeTest, applyModesAndFilter)
{
	IMap::StartPositions starts;
	starts.push_back({ "1", "1", StartPositionModes::BOTH });
	starts.push_back({ "2", "2", StartPositionModes::BOTH });
	std::vector<StartPositionModes::Type> modes;
	modes.push_back(StartPositionModes::BOTH);
	modes.push_back(StartPositionModes::MULTIPLAYER);
	StartPositionModes::applyStartModes(starts, modes);
	EXPECT_EQ(StartPositionModes::BOTH, starts[0]._mode);
	EXPECT_EQ(StartPositionModes::MULTIPLAYER, starts[1]._mode);

	int mp = 0;
	int sp = 0;
	for (const IMap::StartPosition& start : starts) {
		if (StartPositionModes::allowsMultiplayer(start._mode))
			++mp;
		if (StartPositionModes::allowsSingleplayer(start._mode))
			++sp;
	}
	EXPECT_EQ(2, mp);
	EXPECT_EQ(1, sp);
}

namespace {
class StartPositionMapStub : public IMap {
public:
	explicit StartPositionMapStub (StartPositions starts)
	{
		_startPositions = std::move(starts);
	}
	void update (uint32_t) override {}
	bool isActive () const override { return false; }
	void restart (uint32_t) override {}
	int getMapWidth () const override { return 0; }
	int getMapHeight () const override { return 0; }
};
}

TEST(StartPositionModeTest, sessionFilterIgnoresIneligibleStarts)
{
	IMap::StartPositions starts;
	starts.push_back({ "1", "1", StartPositionModes::BOTH });
	starts.push_back({ "8", "3", StartPositionModes::MULTIPLAYER });
	starts.push_back({ "4", "2", StartPositionModes::SINGLEPLAYER });
	const StartPositionMapStub map(starts);

	EXPECT_EQ(2, map.getSessionStartPositionCount(false));
	EXPECT_EQ(2, map.getSessionStartPositionCount(true));

	float x, y;
	ASSERT_TRUE(map.getSessionStartPosition(0, false, x, y));
	EXPECT_FLOAT_EQ(1.0f, x);
	EXPECT_FLOAT_EQ(1.0f, y);
	ASSERT_TRUE(map.getSessionStartPosition(1, false, x, y));
	EXPECT_FLOAT_EQ(4.0f, x);
	EXPECT_FLOAT_EQ(2.0f, y);

	ASSERT_TRUE(map.getSessionStartPosition(0, true, x, y));
	EXPECT_FLOAT_EQ(1.0f, x);
	EXPECT_FLOAT_EQ(1.0f, y);
	ASSERT_TRUE(map.getSessionStartPosition(1, true, x, y));
	EXPECT_FLOAT_EQ(8.0f, x);
	EXPECT_FLOAT_EQ(3.0f, y);
}
