#pragma once

#include "campaign/persister/SQLitePersister.h"

class CavePackerSQLitePersister: public SQLitePersister {
public:
	explicit CavePackerSQLitePersister (const std::string& filename);
	virtual ~CavePackerSQLitePersister ();
};
