#include "CavePackerSQLitePersister.h"
#include "engine/common/campaign/persister/GameStateSQLite.h"

CavePackerSQLitePersister::CavePackerSQLitePersister (const std::string& filename) :
		SQLitePersister(filename)
{
	Transaction t(*_gameState);
	if (_gameState->exec("UPDATE " TABLE_GAMEMAPS " SET campaignid = 'xsokoban', mapid = 'xsokoban' || mapid WHERE campaignid = 'tutorial' AND mapid LIKE '00%'"))
		info(LOG_STORAGE, "updated the gamestate");
	else
		error(LOG_STORAGE, "failed to update the gamestate");
}

CavePackerSQLitePersister::~CavePackerSQLitePersister ()
{
}
