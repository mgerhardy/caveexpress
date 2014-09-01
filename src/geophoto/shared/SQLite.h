#pragma once

#include "common/Compiler.h"
GCC_DIAG_OFF(cast-qual)
GCC_DIAG_OFF(cast-align)
#include <sqlite3.h>
GCC_DIAG_ON(cast-align)
GCC_DIAG_ON(cast-qual)
#include <string>

class SQLiteStatement {
protected:
	sqlite3_stmt *_statement;
public:
	SQLiteStatement ();
	virtual ~SQLiteStatement ();

	void init (sqlite3_stmt *statement);

	// the index is starting at 1 here
	bool bindText (int index, const std::string& value);
	bool bindInt (int index, int value);

	// the index is starting at 0 here
	std::string getText (int index);
	int getInt (int index);

	int step (bool reset = false);

	bool finish ();

	inline operator bool () const
	{
		return _statement != nullptr;
	}
};

class SQLite {
protected:
	std::string _fileName;
	sqlite3 *_db;
public:
	SQLite (const std::string& fileName = "file::memory:?cache=shared");
	virtual ~SQLite ();

	bool open ();
	bool init (const std::string& structure);

	bool exec (const std::string& statement);
	void prepare (SQLiteStatement& stmt, const std::string& statement);
	std::string escape (const std::string& value) const;
};
