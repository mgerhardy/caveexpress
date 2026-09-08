#pragma once

#include <string>

namespace lobby {

const char *const HOST_SUFFIX = " (host)";
const char *const SPECTATING_SUFFIX = " (watching)";
const char *const UNNAMED_PLAYER = "Player";
const char *const SESSION_LOBBY = "lobby";
const char *const SESSION_IN_GAME = "in game";

inline std::string formatPlayerName (const std::string& name, bool isHost, bool spectating = false)
{
	const std::string display = name.empty() ? UNNAMED_PLAYER : name;
	std::string formatted = isHost ? display + HOST_SUFFIX : display;
	if (spectating)
		formatted += SPECTATING_SUFFIX;
	return formatted;
}

inline const char* sessionPhaseLabel (bool inGame)
{
	return inGame ? SESSION_IN_GAME : SESSION_LOBBY;
}

const int MIN_SESSION_PLAYERS = 2;

inline int clampSessionMaxPlayers (int value, int hardMax)
{
	if (value < MIN_SESSION_PLAYERS)
		return MIN_SESSION_PLAYERS;
	if (value > hardMax)
		return hardMax;
	return value;
}

/** Start as soon as the lobby has reached the host's max player setting. */
inline bool shouldAutoStartMatch (bool multiplayer, bool matchStarted, int waitingCount, int maxPlayers)
{
	if (!multiplayer || matchStarted)
		return false;
	return waitingCount >= maxPlayers;
}

struct OverlayVisibility {
	bool startButton = false;
	bool waitLabel = false;
	bool leaveButton = false;
};

/**
 * Wait-overlay chrome. Single-player and an already-started match show nothing.
 * Multiplayer host sees Start + Leave; other clients see Waiting + Leave.
 */
inline OverlayVisibility overlayVisibility (bool multiplayer, bool mapStarted, bool isServerHost)
{
	OverlayVisibility v;
	if (!multiplayer || mapStarted)
		return v;
	v.startButton = isServerHost;
	v.waitLabel = !isServerHost;
	v.leaveButton = true;
	return v;
}

inline bool isVisible (const OverlayVisibility& v)
{
	return v.startButton || v.waitLabel || v.leaveButton;
}

struct AfterMatchUi {
	bool disconnect = true;
	bool popMain = false;
	bool popMapWindow = true;
};

/** Single-player leaves the session. Multiplayer stays on the map window. */
inline AfterMatchUi afterMatchUi (bool multiplayer, bool failed)
{
	AfterMatchUi ui;
	if (multiplayer) {
		ui.disconnect = false;
		ui.popMain = false;
		ui.popMapWindow = false;
		return ui;
	}
	ui.disconnect = true;
	ui.popMain = failed;
	ui.popMapWindow = !failed;
	return ui;
}

}
