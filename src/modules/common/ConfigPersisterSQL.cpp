#include "ConfigPersisterSQL.h"
#include "common/Log.h"
#include "common/Application.h"
#include "common/System.h"

#define TABLE_NAME "config"

ConfigPersisterSQL::ConfigPersisterSQL() :
		_sqlite(System.getDatabaseDirectory() + Singleton<Application>::getInstance().getName() + ".sqlite")
{
	_state = _sqlite.open();
	if (!_state) {
		Log::error(LOG_COMMON, "failed to open the config storage");
		return;
	}
	Log::info(LOG_COMMON, "use %s as config database file", _sqlite.getFilename().c_str());
}

void ConfigPersisterSQL::init ()
{
	_sqlite.exec("CREATE TABLE IF NOT EXISTS " TABLE_NAME " (Name VARCHAR(255) NOT NULL PRIMARY KEY, Value VARCHAR(255))");

	SQLiteStatement stmt;
	_sqlite.prepare(stmt, "SELECT Name, Value FROM " TABLE_NAME ";");
	if (!stmt) {
		Log::error(LOG_COMMON, "failed to load the config values");
		return;
	}

	for (;;) {
		const int s = stmt.step();
		if (s == SQLITE_ROW) {
			const std::string& var = stmt.getText(0);
			const std::string& value = stmt.getText(1);
			_configVarMap[var] = value;
		} else if (s == SQLITE_DONE) {
			Log::info(LOG_COMMON, "loaded all config values: %i", (int)_configVarMap.size());
			break;
		} else {
			Log::error(LOG_COMMON, "SQL step error in config loading");
			return;
		}
	}
}

void ConfigPersisterSQL::save (const std::map<std::string, ConfigVarPtr>& configVars)
{
	if (!_state) {
		Log::error(LOG_COMMON, "no config storage loaded");
		return;
	}

	if (configVars.empty()) {
		Log::error(LOG_COMMON, "no config variables to save");
		return;
	}

	Transaction t(_sqlite);

	_sqlite.exec("DELETE FROM " TABLE_NAME);

	SQLiteStatement stmt;
	_sqlite.prepare(stmt, "INSERT INTO " TABLE_NAME " (Name, Value) VALUES (?, ?)");
	if (!stmt) {
		Log::error(LOG_COMMON, "failed to save the config values");
		return;
	}

	for (std::map<std::string, ConfigVarPtr>::const_iterator i = configVars.begin(); i != configVars.end(); ++i) {
		const ConfigVarPtr &configVar = i->second;
		const std::string& name = configVar->getName();
		const std::string& value = configVar->getValue();
		stmt.bindText(1, name);
		stmt.bindText(2, value);
		stmt.step(true);
		Log::info(LOG_COMMON, "save config var %s with value %s", name.c_str(), value.c_str());
	}
	stmt.finish();

	getSystem().syncFiles();
}

std::string ConfigPersisterSQL::getValue (const std::string& name) const
{
	KeyValueMap::const_iterator i = _configVarMap.find(name);
	if (i == _configVarMap.end())
		return "";
	return i->second;
}

void ConfigPersisterSQL::getVars (std::vector<std::string>& vars) const
{
	for (std::map<std::string, std::string>::const_iterator i = _configVarMap.begin(); i != _configVarMap.end(); ++i) {
		vars.push_back(i->first);
	}
}
