#include "GameStateSQLite.h"
#include "client/round/GameRound.h"
#include "common/System.h"
#include "shared/SQLite.h"
#include "shared/Logger.h"
#include <sstream>

#define TABLE_GAMESTATE "gamestate"

GameStateSQLite::GameStateSQLite () :
		SQLite(System.getDatabaseDirectory() + "gamestate.sqlite")
{
	if (!open())
		System.exit("Could not open gamestate database", 1);
	else
		info(LOG_CLIENT, "loaded gamestate database");

	std::stringstream ss;
	ss << "CREATE TABLE IF NOT EXISTS " TABLE_GAMESTATE " (";
	ss << "foobar TEXT DEFAULT ''";
	ss << ");";
	if (!exec(ss.str()))
		System.exit("Could not create initial gamestate tables", 1);
}

bool GameStateSQLite::saveRound (const GameRound& round)
{
	return false;
}

bool GameStateSQLite::loadRound (GameRound& round)
{
	return false;
}
