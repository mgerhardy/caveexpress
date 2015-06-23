#include "SQLite.h"
#include "common/Log.h"
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
	const int retVal = sqlite3_bind_text(_statement, index, value.c_str(), value.size(), SQLITE_TRANSIENT);
	if (retVal == SQLITE_OK)
		return true;

	_error = sqlite3_errstr(retVal);
	Log::error2(LOG_STORAGE, "can't bind text: %s (%s)", value.c_str(), _error.c_str());
	return false;
}

bool SQLiteStatement::bindInt (int index, int value)
{
	const int retVal = sqlite3_bind_int(_statement, index, value);
	if (retVal == SQLITE_OK)
		return true;
	_error = sqlite3_errstr(retVal);
	Log::error2(LOG_STORAGE, "can't bind int: %i (%s)", value, _error.c_str());
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
	if (_statement == nullptr) {
		return false;
	}

	const int retVal = sqlite3_finalize(_statement);
	if (retVal != SQLITE_OK) {
		_error = sqlite3_errstr(retVal);
		Log::error(LOG_STORAGE, _error);
	}
	_statement = nullptr;
	return true;
}

int SQLiteStatement::step (bool reset)
{
	const int retVal = sqlite3_step(_statement);
	if (retVal != SQLITE_DONE && retVal != SQLITE_ROW) {
		_error = sqlite3_errstr(retVal);
		Log::error2(LOG_STORAGE, "could not step: %s", _error.c_str());
	}

	//sqlite3_clear_bindings(_statement);
	if (reset) {
		const int resetRetVal = sqlite3_reset(_statement);
		if (resetRetVal != SQLITE_OK) {
			_error = sqlite3_errstr(retVal);
			Log::error2(LOG_STORAGE, "could not reset: %s", _error.c_str());
		}
	}

	return retVal;
}

SQLite::SQLite (const std::string& fileName) :
		_fileName(fileName), _db(nullptr)
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
		_error = sqlite3_errmsg(_db);
		Log::error2(LOG_STORAGE, "Can't open database '%s': %s", _fileName.c_str(), _error.c_str());
		sqlite3_close(_db);
		_db = nullptr;
		return false;
	}

	//exec("PRAGMA synchronous=OFF");

	return true;
}

bool SQLite::prepare (SQLiteStatement& s, const std::string& statement)
{
	sqlite3_stmt *stmt;
	const char *pzTest;
	Log::debug2(LOG_STORAGE, "Statement: %s", statement.c_str());
	const int rc = sqlite3_prepare_v2(_db, statement.c_str(), statement.size(), &stmt, &pzTest);
	if (rc != SQLITE_OK) {
		const char *str = sqlite3_errmsg(_db);
		_error = str;
		Log::error2(LOG_STORAGE, "failed to prepare the insert statement: %s", str);
		s.init(nullptr);
		return false;
	}

	s.init(stmt);
	return true;
}

bool SQLite::exec (const std::string& statement)
{
	char *zErrMsg = nullptr;
	Log::debug2(LOG_STORAGE, "Statement: %s", statement.c_str());
	const int rc = sqlite3_exec(_db, statement.c_str(), 0, 0, &zErrMsg);
	if (rc != SQLITE_OK) {
		if (zErrMsg != nullptr) {
			_error = std::string(zErrMsg);
			Log::error2(LOG_STORAGE, "SQL error: %s", zErrMsg);
		}
		sqlite3_free(zErrMsg);
		return false;
	}

	return true;
}
