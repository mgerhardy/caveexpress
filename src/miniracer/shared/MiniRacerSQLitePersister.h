#pragma once

#include "campaign/persister/SQLitePersister.h"

namespace miniracer {

class MiniRacerSQLitePersister: public SQLitePersister {
public:
	explicit MiniRacerSQLitePersister (const std::string& filename);
	virtual ~MiniRacerSQLitePersister ();
};

}
