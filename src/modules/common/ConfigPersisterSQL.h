#pragma once

#include "common/SQLite.h"
#include "common/IConfigPersister.h"
#include <map>

class ConfigPersisterSQL: public IConfigPersister {
private:
	SQLite _sqlite;
	bool _state;
	typedef std::map<std::string, std::string> KeyValueMap;
	KeyValueMap _configVarMap;
public:
	ConfigPersisterSQL (const std::string& file);

	void init () override;

	void save (const std::map<std::string, ConfigVarPtr>& configVars) override;
	std::string getValue (const std::string& name) const override;
	void getVars (std::vector<std::string>& vars) const override;
};
