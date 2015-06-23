#include "CavePackerSQLitePersister.h"

namespace cavepacker {

CavePackerSQLitePersister::CavePackerSQLitePersister (const std::string& filename) :
		SQLitePersister(filename)
{
	Transaction t(*this);
	if (exec("UPDATE " TABLE_GAMEMAPS " SET campaignid = 'xsokoban', mapid = 'xsokoban' || mapid WHERE campaignid = 'tutorial' AND mapid LIKE '00%'")) {
		exec("INSERT OR REPLACE INTO " TABLE_LIVES " (campaignid, lives, version) VALUES ('xsokoban', 3, '1');");
		Log::info2(LOG_STORAGE, "updated the gamestate");
	} else {
		Log::error2(LOG_STORAGE, "failed to update the gamestate");
	}
}

CavePackerSQLitePersister::~CavePackerSQLitePersister ()
{
}

}
