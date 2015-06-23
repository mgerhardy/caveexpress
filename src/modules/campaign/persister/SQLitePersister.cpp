#include "SQLitePersister.h"
#include "campaign/Campaign.h"
#include "common/FileSystem.h"
#include "common/System.h"
#include "common/SQLite.h"
#include "common/Log.h"
#include "common/Application.h"
#include "common/ExecutionTime.h"
#include <sstream>

SQLitePersister::SQLitePersister (const std::string& filename) :
		IGameStatePersister(), SQLite(filename)
{
}

SQLitePersister::~SQLitePersister ()
{
}

bool SQLitePersister::init () {
	if (!open())
		System.exit("Could not open gamestate database", 1);
	else
		Log::info(LOG_CAMPAIGN, "loaded gamestate database");

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

	Log::info(LOG_CAMPAIGN, "use " + getFilename() + " as gamestate database file");

	if (!exec(ss.str()))
		System.exit("Could not create initial gamestate tables", 1);

	_activeCampaign = loadActiveCampaign();
	return true;
}

bool SQLitePersister::saveCampaign (Campaign* campaign)
{
	_activeCampaign = campaign->getId();
	return updateCampaign(campaign);
}

bool SQLitePersister::reset ()
{
	_activeCampaign = DEFAULT_CAMPAIGN;
	return true;
}

bool SQLitePersister::resetCampaign (Campaign* campaign)
{
	_activeCampaign = campaign->getId();
	return resetState(campaign);
}

bool SQLitePersister::activateCampaign (const Campaign* campaign)
{
	return activateCampaign(campaign->getId());
}

std::string SQLitePersister::loadActiveCampaign ()
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
		Log::error(LOG_STORAGE, "error loading activecampaign");
	}

	return DEFAULT_CAMPAIGN;
}

bool SQLitePersister::activateCampaign (const std::string& id)
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

bool SQLitePersister::updateCampaign (Campaign* campaign)
{
	Log::info(LOG_CAMPAIGN, "update campaign progress in database for " + campaign->getId());
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
		Log::info(LOG_CAMPAIGN, "updated campaign progress in database for " + campaign->getId());

	getSystem().syncFiles();

	return savedLives;
}

void SQLitePersister::saveCampaignMapParameters (const CampaignMap* map, SQLiteStatement& stmt)
{
	stmt.bindText(2, map->getId());
	stmt.bindInt(3, map->isLocked() ? 1 : 0);
	stmt.bindInt(4, map->getTime());
	stmt.bindInt(5, map->getFinishPoints());
	stmt.bindInt(6, map->getStars());
}

bool SQLitePersister::deleteMaps (Campaign* campaign)
{
	SQLiteStatement stmt;
	prepare(stmt, "DELETE FROM " TABLE_GAMEMAPS " WHERE campaignid = ?;");
	if (!stmt)
		return false;
	stmt.bindText(1, campaign->getId());
	stmt.step();
	return true;
}

bool SQLitePersister::loadCampaign (Campaign* campaign)
{
	_activeCampaign = campaign->getId();
	const uint8_t lives = loadLives(campaign->getId());
	if (lives == 0) {
		Log::error(LOG_STORAGE, "no live entry for " + campaign->getId());
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
			Log::error(LOG_STORAGE, "SQL step error in loadCampaign");
			return false;
		}
	}

	return campaign->getLives() > 0;
}

void SQLitePersister::loadCampaignMapParameters(CampaignMap* map, SQLiteStatement& stmt)
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
	Log::trace(LOG_STORAGE, "%s", map->toString().c_str());
}

bool SQLitePersister::saveLives (uint8_t lives, const std::string& campaignId)
{
	SQLiteStatement stmt;
	prepare(stmt, "INSERT OR REPLACE INTO " TABLE_LIVES " (campaignid, lives, version) VALUES (?, ?, ?);");
	if (!stmt)
		return false;

	stmt.bindText(1, campaignId);
	stmt.bindInt(2, lives);
	stmt.bindText(3, Singleton<Application>::getInstance().getVersion());
	stmt.step();
	Log::info2(LOG_CAMPAIGN, "update lives in database %i", static_cast<int>(lives));
	return true;
}

bool SQLitePersister::resetState (Campaign* campaign)
{
	Transaction t(*this);
	const int lives = INITIAL_LIVES;
	if (saveLives(lives, campaign->getId()) && deleteMaps(campaign) && activateCampaign(campaign)) {
		campaign->setLives(lives);

		getSystem().syncFiles();
		return true;
	}
	Log::error2(LOG_STORAGE, "error reseting the state");
	return false;
}

uint8_t SQLitePersister::loadLives (const std::string& campaignId)
{
	SQLiteStatement stmt;
	prepare(stmt, "SELECT lives FROM " TABLE_LIVES " WHERE campaignid = ?;");
	if (!stmt) {
		Log::info2(LOG_STORAGE, "no lives entry for %s", campaignId.c_str());
		return 0;
	}

	stmt.bindText(1, campaignId);

	const int s = stmt.step();
	if (s == SQLITE_ROW) {
		const int lives = stmt.getInt(0);
		Log::info(LOG_STORAGE, "got " + string::toString(lives) + " lives for campaign " + campaignId);
		return lives;
	} else if (s != SQLITE_DONE) {
		Log::error2(LOG_STORAGE, "error loading lives");
	}

	return 0;
}
