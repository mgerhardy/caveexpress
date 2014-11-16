#pragma once

#include "engine/common/SQLite.h"
#include <stdint.h>

// forward decl
class Campaign;
class CampaignMap;

#define TABLE_GAMESTATE "gamestate"
#define TABLE_GAMEMAPS "maps"
#define TABLE_LIVES "lives"

class GameStateSQLite: public SQLite {
protected:
	virtual bool deleteMaps (Campaign* campaign);
	virtual bool activateCampaign (const Campaign* campaign);
	virtual bool activateCampaign (const std::string& id);

	virtual bool saveLives (uint8_t lives, const std::string& campaignId);
	virtual void loadCampaignMapParameters(CampaignMap* map, SQLiteStatement& stmt);
public:
	GameStateSQLite (const std::string& filename);

	virtual bool updateCampaign (Campaign* campaign);
	virtual std::string getActiveCampaign ();
	virtual bool loadCampaign (Campaign* campaign);

	virtual bool resetState (Campaign* campaign);

	virtual uint8_t loadLives (const std::string& campaignId);
};
