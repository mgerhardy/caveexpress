#include "tests/TestShared.h"
#ifdef scroll
#undef scroll
#endif
#include "ui/UI.h"
#include "ui/windows/IUIMapWindow.h"
#include "ui/nodes/IUINodeMap.h"
#include "ui/nodes/UINode.h"
#include "common/Commands.h"
#include "common/CommandSystem.h"
#include "common/LobbyPlayers.h"
#include "common/MapSettings.h"
#include "caveexpress/client/ui/windows/UIMapFailedWindow.h"
#include "caveexpress/shared/CaveExpressMapFailedReasons.h"
#include "common/ThemeType.h"
#include "common/ConfigManager.h"
#include <string>
#include <vector>

class MapWindowLobbyTest: public AbstractTest {
protected:
	IUIMapWindow* _window = nullptr;
	IUINodeMap* _nodeMap = nullptr;

	void SetUp () override
	{
		AbstractTest::SetUp();
		UI::get().init(_serviceProvider, _eventHandler, _testFrontend);
		_window = static_cast<IUIMapWindow*>(UI::get().getWindow(UI_WINDOW_MAP));
		ASSERT_NE(nullptr, _window);
		_nodeMap = static_cast<IUINodeMap*>(_window->getNode(UINODE_MAP));
		ASSERT_NE(nullptr, _nodeMap);
	}

	void TearDown () override
	{
		_window = nullptr;
		_nodeMap = nullptr;
		UI::get().shutdown();
		AbstractTest::TearDown();
	}

	UINode* requireNode (const char* id)
	{
		UINode* node = _window->getNode(id);
		EXPECT_NE(nullptr, node) << id;
		return node;
	}
};

TEST_F(MapWindowLobbyTest, singlePlayerMapWindowHasNoLobbyChrome)
{
	ASSERT_FALSE(_serviceProvider.getNetwork().isMultiplayer());
	EXPECT_TRUE(_nodeMap->getMap().isStarted()) << "single-player ClientMap reports started immediately";
	EXPECT_TRUE(_nodeMap->initWaitingForPlayer()) << "SP must auto-start, not sit in the lobby";

	_window->initWaitingForPlayers(true);
	EXPECT_FALSE(requireNode("startbutton")->isVisible());
	EXPECT_FALSE(requireNode("waitpanel")->isVisible());
	EXPECT_FALSE(requireNode("leavebutton")->isVisible());

	_window->start();
	EXPECT_FALSE(requireNode("startbutton")->isVisible());
	EXPECT_FALSE(requireNode("waitpanel")->isVisible());
	EXPECT_FALSE(requireNode("leavebutton")->isVisible());
}

TEST_F(MapWindowLobbyTest, multiplayerHostShowsStartAndLeave)
{
	_serviceProvider.updateNetwork(true);
	ASSERT_TRUE(_serviceProvider.getNetwork().isMultiplayer());
	EXPECT_FALSE(_nodeMap->initWaitingForPlayer());

	_window->initWaitingForPlayers(true);
	EXPECT_TRUE(requireNode("startbutton")->isVisible());
	EXPECT_FALSE(requireNode("waitpanel")->isVisible());
	EXPECT_TRUE(requireNode("leavebutton")->isVisible());

	_window->start();
	EXPECT_FALSE(requireNode("startbutton")->isVisible());
	EXPECT_FALSE(requireNode("waitpanel")->isVisible());
	EXPECT_FALSE(requireNode("leavebutton")->isVisible());
}

TEST_F(MapWindowLobbyTest, multiplayerClientShowsWaitingAndLeave)
{
	_serviceProvider.updateNetwork(true);
	_window->initWaitingForPlayers(false);
	EXPECT_FALSE(requireNode("startbutton")->isVisible());
	EXPECT_TRUE(requireNode("waitpanel")->isVisible());
	EXPECT_TRUE(requireNode("leavebutton")->isVisible());
}

TEST_F(MapWindowLobbyTest, playerListKeepsHostMarker)
{
	std::vector<std::string> names;
	names.push_back(lobby::formatPlayerName("Alice", true));
	names.push_back(lobby::formatPlayerName("Bob", false));
	_nodeMap->setPlayerList(names);
	ASSERT_EQ(2u, _nodeMap->getPlayerList().size());
	EXPECT_EQ(std::string("Alice") + lobby::HOST_SUFFIX, _nodeMap->getPlayerList()[0]);
	EXPECT_EQ("Bob", _nodeMap->getPlayerList()[1]);
}

TEST_F(MapWindowLobbyTest, leaveButtonIsWiredToDisconnect)
{
	_serviceProvider.updateNetwork(true);
	_window->initWaitingForPlayers(false);
	UINode* leave = requireNode("leavebutton");
	ASSERT_TRUE(leave->isVisible());
	EXPECT_TRUE(Commands.commandExists(CMD_CL_DISCONNECT));
}

TEST_F(MapWindowLobbyTest, leaveButtonPopsMapWindow)
{
	_serviceProvider.updateNetwork(true);
	UI::get().initStack();
	ASSERT_NE(nullptr, UI::get().push(UI_WINDOW_MAP));
	ASSERT_NE(nullptr, UI::get().getFrontWindow());
	EXPECT_EQ(std::string(UI_WINDOW_MAP), UI::get().getFrontWindow()->getId());

	ClientMap& clientMap = _nodeMap->getMap();
	ASSERT_TRUE(clientMap.load("ice-01", "Ice 01"));
	clientMap.setSetting(msn::WIDTH, "16");
	clientMap.setSetting(msn::HEIGHT, "12");
	ASSERT_TRUE(clientMap.isActive()) << "Leave must close an active client map or onPop opens Options";

	_window->initWaitingForPlayers(true);
	UINode* leave = requireNode("leavebutton");
	ASSERT_TRUE(leave->isVisible());
	ASSERT_TRUE(leave->onMouseLeftRelease(0, 0));

	ASSERT_NE(nullptr, UI::get().getFrontWindow());
	EXPECT_NE(std::string(UI_WINDOW_MAP), UI::get().getFrontWindow()->getId())
		<< "host Leave closed the listen server but used to leave the lobby UI up";
	EXPECT_NE(std::string(UI_WINDOW_OPTIONS), UI::get().getFrontWindow()->getId())
		<< "Leave must not treat the map as paused";
	EXPECT_FALSE(clientMap.isActive());
}

TEST_F(MapWindowLobbyTest, failedWindowSinglePlayerKeepsRetry)
{
	auto* failed = static_cast<caveexpress::UIMapFailedWindow*>(UI::get().getWindow(UI_WINDOW_MAPFAILED));
	ASSERT_NE(nullptr, failed);
	failed->updateReason(false, caveexpress::MapFailedReasons::FAILED_HITPOINTS, ThemeTypes::ROCK);
	UINode* retry = failed->getNode("retrybutton");
	UINode* lobbyBtn = failed->getNode("lobbybutton");
	ASSERT_NE(nullptr, retry);
	ASSERT_NE(nullptr, lobbyBtn);
	EXPECT_TRUE(retry->isVisible());
	EXPECT_FALSE(lobbyBtn->isVisible());
}

TEST_F(MapWindowLobbyTest, failedWindowMultiplayerReturnsToLobby)
{
	auto* failed = static_cast<caveexpress::UIMapFailedWindow*>(UI::get().getWindow(UI_WINDOW_MAPFAILED));
	ASSERT_NE(nullptr, failed);
	failed->updateReason(true, caveexpress::MapFailedReasons::FAILED_HITPOINTS, ThemeTypes::ROCK);
	UINode* retry = failed->getNode("retrybutton");
	UINode* lobbyBtn = failed->getNode("lobbybutton");
	ASSERT_NE(nullptr, retry);
	ASSERT_NE(nullptr, lobbyBtn);
	EXPECT_FALSE(retry->isVisible());
	EXPECT_TRUE(lobbyBtn->isVisible());
	EXPECT_TRUE(failed->isFullscreen()) << "fail overlay must hide the map so it does not keep drawing";
}

TEST_F(MapWindowLobbyTest, overlayOverStartedMapUsesUiBindings)
{
	_window->start();
	EXPECT_EQ(BINDINGS_MAP, Config.getBindingsSpace());
	_window->onPushedOver();
	EXPECT_EQ(BINDINGS_UI, Config.getBindingsSpace())
			<< "finish/fail/options must not keep spectate/drop/move bindings";
}

TEST_F(MapWindowLobbyTest, lobbyStaysOnUiBindingsWhenMapBecomesActive)
{
	_serviceProvider.updateNetwork(true);
	_window->start();
	ASSERT_EQ(BINDINGS_MAP, Config.getBindingsSpace());
	_window->initWaitingForPlayers(true);
	EXPECT_EQ(BINDINGS_UI, Config.getBindingsSpace());
	_window->onActive();
	EXPECT_EQ(BINDINGS_UI, Config.getBindingsSpace())
			<< "after a match the map is still started; lobby chrome must keep UI keys";
}

TEST_F(MapWindowLobbyTest, startedMatchUsesMapBindingsWhenFront)
{
	_window->start();
	_window->onActive();
	EXPECT_EQ(BINDINGS_MAP, Config.getBindingsSpace());
}

TEST_F(MapWindowLobbyTest, finishedWindowForcesUiBindings)
{
	auto* finished = UI::get().getWindow(UI_WINDOW_MAPFINISHED);
	ASSERT_NE(nullptr, finished);
	_window->start();
	ASSERT_EQ(BINDINGS_MAP, Config.getBindingsSpace());
	finished->onActive();
	EXPECT_EQ(BINDINGS_UI, Config.getBindingsSpace());
}

TEST_F(MapWindowLobbyTest, failedWindowForcesUiBindings)
{
	auto* failed = UI::get().getWindow(UI_WINDOW_MAPFAILED);
	ASSERT_NE(nullptr, failed);
	_window->start();
	ASSERT_EQ(BINDINGS_MAP, Config.getBindingsSpace());
	failed->onActive();
	EXPECT_EQ(BINDINGS_UI, Config.getBindingsSpace());
}

TEST_F(MapWindowLobbyTest, createServerHasMaxPlayersSpinner)
{
	UIWindow* create = UI::get().getWindow(UI_WINDOW_CREATE_SERVER);
	ASSERT_NE(nullptr, create);
	UINode* spinner = create->getNode("maxplayers");
	ASSERT_NE(nullptr, spinner) << "Create Server must expose a max players control";
}
