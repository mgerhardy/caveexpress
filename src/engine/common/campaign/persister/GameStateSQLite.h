#pragma once

#include "engine/common/SQLite.h"
#include <stdint.h>

// forward decl
class Campaign;

#define TABLE_GAMESTATE "gamestate"
#define TABLE_GAMEMAPS "maps"
#define TABLE_LIVES "lives"

class GameStateSQLite: public SQLite {
private:
	bool deleteMaps (Campaign* campaign);
	bool activateCampaign (const Campaign* campaign);
	bool activateCampaign (const std::string& id);

	bool saveLives (uint8_t lives, const std::string& campaignId);
public:
	GameStateSQLite (const std::string& filename);

	bool updateCampaign (Campaign* campaign);
	std::string getActiveCampaign ();
	bool loadCampaign (Campaign* campaign);

	bool resetState (Campaign* campaign);

	uint8_t loadLives (const std::string& campaignId);
};
