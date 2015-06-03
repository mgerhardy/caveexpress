#pragma once

#include "campaign/persister/SQLitePersister.h"

class CavePackerSQLitePersister: public SQLitePersister {
public:
	CavePackerSQLitePersister (const std::string& filename);
	virtual ~CavePackerSQLitePersister ();
};
