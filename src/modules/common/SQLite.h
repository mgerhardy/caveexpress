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
	std::string _error;
	sqlite3_stmt *_statement;
	sqlite3 *_db;
public:
	SQLiteStatement ();
	virtual ~SQLiteStatement ();

	void init (sqlite3_stmt *statement, sqlite3 *db);

	// the index is starting at 1 here
	bool bindText (int index, const std::string& value);
	bool bindInt (int index, int value);

	// return the error string in case an error occured.
	const std::string& getError () const;

	// the index is starting at 0 here
	std::string getText (int index);
	int getInt (int index);

	// returns the sqlite state code
	// return values other than SQLITE_DONE and SQLITE_ROW are errors
	int step (bool reset = false);

	bool finish ();

	inline operator bool () const
	{
		return _statement != nullptr;
	}
};

inline const std::string& SQLiteStatement::getError () const
{
	return _error;
}

class SQLite {
protected:
	std::string _fileName;
	std::string _error;
	sqlite3 *_db;
public:
	explicit SQLite (const std::string& fileName = "");
	virtual ~SQLite ();

	const std::string& getFilename () const;

	// return the error string in case an error occured.
	const std::string& getError () const;

	bool open ();
	bool init (const std::string& structure);

	bool exec (const std::string& statement);
	bool prepare (SQLiteStatement& stmt, const std::string& statement);
};

inline const std::string& SQLite::getFilename () const
{
	return _fileName;
}

inline const std::string& SQLite::getError () const
{
	return _error;
}

class Transaction {
private:
	SQLite& _sqlite;
public:
	explicit Transaction(SQLite& sqlite) : _sqlite(sqlite)
	{
		_sqlite.exec("BEGIN;");
	}

	~Transaction()
	{
		_sqlite.exec("COMMIT;");
	}
};
