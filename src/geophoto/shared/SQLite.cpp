#include "SQLite.h"
#include "shared/Logger.h"
#include <SDL_platform.h>

SQLiteStatement::SQLiteStatement () :
		_statement(nullptr)
{
}

SQLiteStatement::~SQLiteStatement ()
{
	finish();
}

void SQLiteStatement::init (sqlite3_stmt *statement)
{
	_statement = statement;
}

bool SQLiteStatement::bindText (int index, const std::string& value)
{
	if (sqlite3_bind_text(_statement, index, value.c_str(), value.size(), SQLITE_TRANSIENT) == SQLITE_OK)
		return true;

	error(LOG_STORAGE, "can't bind text: " + value);
	return false;
}

bool SQLiteStatement::bindInt (int index, int value)
{
	if (sqlite3_bind_int(_statement, index, value) == SQLITE_OK)
		return true;

	error(LOG_STORAGE, "can't bind int: " + string::toString(value));
	return false;
}

int SQLiteStatement::getInt (int index)
{
	return sqlite3_column_int(_statement, index);
}

std::string SQLiteStatement::getText (int index)
{
	return reinterpret_cast<const char *>(sqlite3_column_text(_statement, index));
}

bool SQLiteStatement::finish ()
{
	if (_statement == nullptr)
		return false;

	sqlite3_finalize(_statement);
	_statement = nullptr;
	return true;
}

int SQLiteStatement::step (bool reset)
{
	const int retVal = sqlite3_step(_statement);
	if (retVal != SQLITE_DONE && retVal != SQLITE_ROW) {
		error(LOG_STORAGE, "could not step");
	}

	//sqlite3_clear_bindings(_statement);
	if (reset)
		sqlite3_reset(_statement);

	return retVal;
}

SQLite::SQLite (const std::string& fileName) :
		_fileName(fileName), _db(0)
{
}

SQLite::~SQLite ()
{
	if (_db)
		sqlite3_close(_db);
}

bool SQLite::init (const std::string& structure)
{
#if 0
	sqlite3 *db;
	int rc = sqlite3_open(_fileName.c_str(), &db);
	if (rc) {
		sqlite3_close(db);
		return false;
	}
#endif
	return true;
}

bool SQLite::open ()
{
	const int rc = sqlite3_open(_fileName.c_str(), &_db);
	if (rc != SQLITE_OK) {
		error(LOG_STORAGE, String::format("Can't open database '%s': %s", _fileName.c_str(), sqlite3_errmsg(_db)));
		sqlite3_close(_db);
		_db = nullptr;
		return false;
	}

	//exec("PRAGMA synchronous=OFF");

	return true;
}

void SQLite::prepare (SQLiteStatement& s, const std::string& statement)
{
	sqlite3_stmt *stmt;
	const char *pzTest;
	debug(LOG_STORAGE, "Statement: " + statement);
	const int rc = sqlite3_prepare_v2(_db, statement.c_str(), statement.size(), &stmt, &pzTest);
	if (rc != SQLITE_OK) {
		error(LOG_STORAGE, String::format("failed to prepare the insert statement: %s", sqlite3_errmsg(_db)));
		s.init(nullptr);
		return;
	}

	s.init(stmt);
}

bool SQLite::exec (const std::string& statement)
{
	char *zErrMsg = 0;
	debug(LOG_STORAGE, "Statement: " + statement);
	const int rc = sqlite3_exec(_db, statement.c_str(), 0, 0, &zErrMsg);
	if (rc != SQLITE_OK) {
		error(LOG_STORAGE, String::format("SQL error: %s", zErrMsg));
		sqlite3_free(zErrMsg);
		return false;
	}

	return true;
}

std::string SQLite::escape (const std::string& value) const
{
	// FIXME: check that no ' is included in value string
	return "'" + value + "'";
}
