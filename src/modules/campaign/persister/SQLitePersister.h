#pragma once

#include "IGameStatePersister.h"
#include "common/SQLite.h"
#include <stdint.h>

// forward decl
class Campaign;
class CampaignMap;

#define TABLE_GAMESTATE "gamestate"
#define TABLE_GAMEMAPS "maps"
#define TABLE_LIVES "lives"

class SQLitePersister: public IGameStatePersister, public SQLite {
protected:
	virtual bool deleteMaps (Campaign* campaign);
	virtual bool activateCampaign (const Campaign* campaign);
	virtual bool activateCampaign (const std::string& id);

	virtual bool saveLives (uint8_t lives, const std::string& campaignId);
	virtual void loadCampaignMapParameters(CampaignMap* map, SQLiteStatement& stmt);
	virtual void saveCampaignMapParameters (const CampaignMap* map, SQLiteStatement& stmt);

	virtual std::string loadActiveCampaign ();
	virtual bool updateCampaign (Campaign* campaign);

	virtual bool resetState (Campaign* campaign);

	virtual uint8_t loadLives (const std::string& campaignId);
public:
	explicit SQLitePersister (const std::string& filename);
	virtual ~SQLitePersister ();

	virtual bool init () override;
	virtual bool reset () override;
	virtual bool saveCampaign (Campaign* campaign) override;
	virtual bool loadCampaign (Campaign* campaign) override;
	virtual bool resetCampaign (Campaign* campaign) override;
};
