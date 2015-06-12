#pragma once

#include "campaign/persister/SQLitePersister.h"

namespace cavepacker {

class CavePackerSQLitePersister: public SQLitePersister {
public:
	explicit CavePackerSQLitePersister (const std::string& filename);
	virtual ~CavePackerSQLitePersister ();
};

}
