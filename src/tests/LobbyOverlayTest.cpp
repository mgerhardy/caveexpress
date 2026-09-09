#include <gtest/gtest.h>
#include "common/LobbyPlayers.h"
#include "common/Spectate.h"
#include <vector>

TEST(LobbyOverlayTest, formatPlayerNameMarksHost)
{
	EXPECT_EQ("Alice", lobby::formatPlayerName("Alice", false));
	EXPECT_EQ(std::string("Alice") + lobby::HOST_SUFFIX, lobby::formatPlayerName("Alice", true));
	EXPECT_EQ(lobby::UNNAMED_PLAYER, lobby::formatPlayerName("", false));
	EXPECT_EQ(std::string(lobby::UNNAMED_PLAYER) + lobby::HOST_SUFFIX, lobby::formatPlayerName("", true));
	EXPECT_EQ(std::string("Bob") + lobby::SPECTATING_SUFFIX, lobby::formatPlayerName("Bob", false, true));
}

TEST(LobbyOverlayTest, clampSessionMaxPlayers)
{
	EXPECT_EQ(2, lobby::clampSessionMaxPlayers(0, 4));
	EXPECT_EQ(2, lobby::clampSessionMaxPlayers(1, 4));
	EXPECT_EQ(2, lobby::clampSessionMaxPlayers(2, 4));
	EXPECT_EQ(4, lobby::clampSessionMaxPlayers(4, 4));
	EXPECT_EQ(4, lobby::clampSessionMaxPlayers(99, 4));
}

TEST(LobbyOverlayTest, shouldAutoStartMatch)
{
	EXPECT_FALSE(lobby::shouldAutoStartMatch(false, false, 2, 2));
	EXPECT_FALSE(lobby::shouldAutoStartMatch(true, true, 2, 2));
	EXPECT_FALSE(lobby::shouldAutoStartMatch(true, false, 1, 2));
	EXPECT_TRUE(lobby::shouldAutoStartMatch(true, false, 2, 2));
	EXPECT_FALSE(lobby::shouldAutoStartMatch(true, false, 2, 4));
	EXPECT_TRUE(lobby::shouldAutoStartMatch(true, false, 4, 4));
}

TEST(LobbyOverlayTest, singlePlayerNeverShowsLobbyChrome)
{
	const lobby::OverlayVisibility beforeStart = lobby::overlayVisibility(false, false, true);
	const lobby::OverlayVisibility afterStart = lobby::overlayVisibility(false, true, true);
	EXPECT_FALSE(lobby::isVisible(beforeStart));
	EXPECT_FALSE(lobby::isVisible(afterStart));
	EXPECT_FALSE(beforeStart.startButton);
	EXPECT_FALSE(beforeStart.waitLabel);
	EXPECT_FALSE(beforeStart.leaveButton);
}

TEST(LobbyOverlayTest, multiplayerHostSeesStartAndLeave)
{
	const lobby::OverlayVisibility v = lobby::overlayVisibility(true, false, true);
	EXPECT_TRUE(lobby::isVisible(v));
	EXPECT_TRUE(v.startButton);
	EXPECT_FALSE(v.waitLabel);
	EXPECT_TRUE(v.leaveButton);
}

TEST(LobbyOverlayTest, multiplayerClientSeesWaitingAndLeave)
{
	const lobby::OverlayVisibility v = lobby::overlayVisibility(true, false, false);
	EXPECT_TRUE(lobby::isVisible(v));
	EXPECT_FALSE(v.startButton);
	EXPECT_TRUE(v.waitLabel);
	EXPECT_TRUE(v.leaveButton);
}

TEST(LobbyOverlayTest, startedMatchHidesLobbyChrome)
{
	const lobby::OverlayVisibility host = lobby::overlayVisibility(true, true, true);
	const lobby::OverlayVisibility client = lobby::overlayVisibility(true, true, false);
	EXPECT_FALSE(lobby::isVisible(host));
	EXPECT_FALSE(lobby::isVisible(client));
}

TEST(LobbyOverlayTest, singlePlayerFailLeavesTheSession)
{
	const lobby::AfterMatchUi ui = lobby::afterMatchUi(false, true);
	EXPECT_TRUE(ui.disconnect);
	EXPECT_TRUE(ui.popMain);
	EXPECT_FALSE(ui.popMapWindow);
}

TEST(LobbyOverlayTest, singlePlayerFinishPopsTheMap)
{
	const lobby::AfterMatchUi ui = lobby::afterMatchUi(false, false);
	EXPECT_TRUE(ui.disconnect);
	EXPECT_FALSE(ui.popMain);
	EXPECT_TRUE(ui.popMapWindow);
}

TEST(LobbyOverlayTest, multiplayerFailAndFinishStayOnTheMap)
{
	const lobby::AfterMatchUi fail = lobby::afterMatchUi(true, true);
	const lobby::AfterMatchUi finish = lobby::afterMatchUi(true, false);
	EXPECT_FALSE(fail.disconnect);
	EXPECT_FALSE(fail.popMain);
	EXPECT_FALSE(fail.popMapWindow);
	EXPECT_FALSE(finish.disconnect);
	EXPECT_FALSE(finish.popMain);
	EXPECT_FALSE(finish.popMapWindow);
}

TEST(LobbyOverlayTest, spectateCycleWrapsAround)
{
	EXPECT_EQ(-1, spectate::wrapIndex(0, 0, 1));
	EXPECT_EQ(0, spectate::wrapIndex(-1, 3, 1));
	EXPECT_EQ(2, spectate::wrapIndex(-1, 3, -1));
	EXPECT_EQ(1, spectate::wrapIndex(0, 3, 1));
	EXPECT_EQ(0, spectate::wrapIndex(2, 3, 1));
	EXPECT_EQ(2, spectate::wrapIndex(0, 3, -1));

	const std::vector<uint16_t> ids = { 10, 20, 30 };
	EXPECT_EQ(0, spectate::cycleId({}, 10, 1));
	EXPECT_EQ(20, spectate::cycleId(ids, 10, 1));
	EXPECT_EQ(10, spectate::cycleId(ids, 30, 1));
	EXPECT_EQ(30, spectate::cycleId(ids, 10, -1));
	EXPECT_EQ(10, spectate::cycleId(ids, 99, 1));
	EXPECT_EQ(30, spectate::cycleId(ids, 99, -1));
}
