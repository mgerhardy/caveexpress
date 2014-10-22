#pragma once

#include "IGameStatePersister.h"

// forward decl
class GameStateSQLite;

class SQLitePersister: public IGameStatePersister {
protected:
	std::string _filename;
	GameStateSQLite *_gameState;
public:
	SQLitePersister (const std::string& filename);
	virtual ~SQLitePersister ();

	bool reset () override;
	bool saveCampaign (Campaign* campaign) override;
	bool loadCampaign (Campaign* campaign) override;
	bool resetCampaign (Campaign* campaign) override;
};
