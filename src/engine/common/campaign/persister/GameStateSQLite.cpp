#include "GameStateSQLite.h"
#include "engine/common/campaign/persister/IGameStatePersister.h"
#include "engine/common/campaign/Campaign.h"
#include "engine/common/System.h"
#include "engine/common/SQLite.h"
#include "engine/common/Logger.h"
#include "engine/common/Application.h"
#include "engine/common/ExecutionTime.h"
#include <sstream>

GameStateSQLite::GameStateSQLite (const std::string& filename) :
		SQLite(filename)
{
	if (!open())
		System.exit("Could not open gamestate database", 1);
	else
		info(LOG_CAMPAIGN, "loaded gamestate database");

	std::stringstream ss;
	ss << "CREATE TABLE IF NOT EXISTS " TABLE_GAMESTATE " (";
	ss << "activecampaign TEXT DEFAULT '" DEFAULT_CAMPAIGN << "', ";
	ss << "version TEXT";
	ss << ");";
	ss << "CREATE TABLE IF NOT EXISTS " TABLE_LIVES " (";
	ss << "campaignid TEXT DEFAULT '" DEFAULT_CAMPAIGN "', ";
	ss << "lives INTEGER DEFAULT " << INITIAL_LIVES << " NOT NULL, ";
	ss << "version TEXT, ";
	ss << "PRIMARY KEY(campaignid)";
	ss << ");";
	ss << "CREATE TABLE IF NOT EXISTS " TABLE_GAMEMAPS " (";
	ss << "campaignid TEXT, ";
	ss << "mapid TEXT, ";
	ss << "locked INTEGER DEFAULT 1 NOT NULL, ";
	ss << "time INTEGER DEFAULT 0 NOT NULL, ";
	ss << "finishPoints INTEGER DEFAULT 0 NOT NULL, ";
	ss << "stars INTEGER DEFAULT 0 NOT NULL, ";
	ss << "version TEXT, ";
	ss << "PRIMARY KEY(campaignid, mapid)";
	ss << ");";

	info(LOG_CAMPAIGN, "use " + getFilename() + " as gamestate database file");

	if (!exec(ss.str()))
		System.exit("Could not create initial gamestate tables", 1);
}

bool GameStateSQLite::activateCampaign (const Campaign* campaign)
{
	return activateCampaign(campaign->getId());
}

std::string GameStateSQLite::getActiveCampaign ()
{
	SQLiteStatement stmt;
	prepare(stmt, "SELECT activecampaign FROM " TABLE_GAMESTATE);
	if (!stmt) {
		return DEFAULT_CAMPAIGN;
	}

	const int s = stmt.step();
	if (s == SQLITE_ROW) {
		return stmt.getText(0);
	} else if (s != SQLITE_DONE) {
		error(LOG_STORAGE, "error loading activecampaign");
	}

	return DEFAULT_CAMPAIGN;
}

bool GameStateSQLite::activateCampaign (const std::string& id)
{
	SQLiteStatement stmt;
	prepare(stmt, "UPDATE " TABLE_GAMESTATE " SET activecampaign = ?, version = ?;");
	if (!stmt)
		return false;
	stmt.bindText(1, id);
	stmt.bindText(2, Singleton<Application>::getInstance().getVersion());
	stmt.step();
	return true;
}

bool GameStateSQLite::updateCampaign (Campaign* campaign)
{
	info(LOG_CAMPAIGN, "update campaign progress in database for " + campaign->getId());
	Transaction t(*this);
	ExecutionTime e("save progress");

	activateCampaign(campaign);

	SQLiteStatement stmt;
	prepare(stmt, "INSERT OR REPLACE INTO " TABLE_GAMEMAPS " (campaignid, mapid, locked, time, finishPoints, stars, version) VALUES (?, ?, ?, ?, ?, ?, ?);");
	if (!stmt)
		return false;

	stmt.bindText(1, campaign->getId());
	stmt.bindText(7, Singleton<Application>::getInstance().getName());
	const Campaign::MapList& maps = campaign->getMaps();
	for (Campaign::MapListConstIter i = maps.begin(); i != maps.end(); ++i) {
		const CampaignMapPtr& map = *i;
		saveCampaignMapParameters(map.get(), stmt);
		stmt.step(true);
	}
	const bool savedLives = saveLives(campaign->getLives(), campaign->getId());
	if (savedLives)
		info(LOG_CAMPAIGN, "updated campaign progress in database for " + campaign->getId());
	return savedLives;
}

void GameStateSQLite::saveCampaignMapParameters (const CampaignMap* map, SQLiteStatement& stmt)
{
	stmt.bindText(2, map->getId());
	stmt.bindInt(3, map->isLocked() ? 1 : 0);
	stmt.bindInt(4, map->getTime());
	stmt.bindInt(5, map->getFinishPoints());
	stmt.bindInt(6, map->getStars());
}

bool GameStateSQLite::deleteMaps (Campaign* campaign)
{
	SQLiteStatement stmt;
	prepare(stmt, "DELETE FROM " TABLE_GAMEMAPS " WHERE campaignid = ?;");
	if (!stmt)
		return false;
	stmt.bindText(1, campaign->getId());
	stmt.step();
	return true;
}

bool GameStateSQLite::loadCampaign (Campaign* campaign)
{
	const uint8_t lives = loadLives(campaign->getId());
	if (lives == 0) {
		error(LOG_STORAGE, "no live entry for " + campaign->getId());
		return false;
	}

	campaign->resetMaps();
	campaign->setLives(lives);
	SQLiteStatement stmt;
	prepare(stmt, "SELECT * FROM " TABLE_GAMEMAPS " WHERE campaignid = ?;");
	if (!stmt)
		return false;

	stmt.bindText(1, campaign->getId());

	for (;;) {
		const int s = stmt.step();
		if (s == SQLITE_ROW) {
			const std::string mapid = stmt.getText(1);
			CampaignMap* map = campaign->getMapById(mapid);
			if (map) {
				loadCampaignMapParameters(map, stmt);
			}
		} else if (s == SQLITE_DONE) {
			break;
		} else {
			error(LOG_STORAGE, "SQL step error in loadCampaign");
			return false;
		}
	}

	return campaign->getLives() > 0;
}

void GameStateSQLite::loadCampaignMapParameters(CampaignMap* map, SQLiteStatement& stmt)
{
	const int locked = stmt.getInt(2);
	const int time = stmt.getInt(3);
	const int finishPoints = stmt.getInt(4);
	const int stars = stmt.getInt(5);
	map->setTime(time);
	map->setFinishPoints(finishPoints);
	map->setStars(stars);
	if (locked == 0)
		map->unlock();
	debug(LOG_STORAGE, map->toString());
}

bool GameStateSQLite::saveLives (uint8_t lives, const std::string& campaignId)
{
	SQLiteStatement stmt;
	prepare(stmt, "INSERT OR REPLACE INTO " TABLE_LIVES " (campaignid, lives, version) VALUES (?, ?, ?);");
	if (!stmt)
		return false;

	stmt.bindText(1, campaignId);
	stmt.bindInt(2, lives);
	stmt.bindText(3, Singleton<Application>::getInstance().getVersion());
	stmt.step();
	info(LOG_CAMPAIGN, "update lives in database " + string::toString(static_cast<int>(lives)));
	return true;
}

bool GameStateSQLite::resetState (Campaign* campaign)
{
	Transaction t(*this);
	const int lives = INITIAL_LIVES;
	if (saveLives(lives, campaign->getId()) && deleteMaps(campaign) && activateCampaign(campaign)) {
		campaign->setLives(lives);
		return true;
	}
	error(LOG_STORAGE, "error reseting the state");
	return false;
}

uint8_t GameStateSQLite::loadLives (const std::string& campaignId)
{
	SQLiteStatement stmt;
	prepare(stmt, "SELECT lives FROM " TABLE_LIVES " WHERE campaignid = ?;");
	if (!stmt) {
		info(LOG_STORAGE, "no lives entry for " + campaignId);
		return 0;
	}

	stmt.bindText(1, campaignId);

	const int s = stmt.step();
	if (s == SQLITE_ROW) {
		const int lives = stmt.getInt(0);
		info(LOG_STORAGE, "got " + string::toString(lives) + " lives for campaign " + campaignId);
		return lives;
	} else if (s != SQLITE_DONE) {
		error(LOG_STORAGE, "error loading lives");
	}

	return 0;
}
