#pragma once

#include "common/IConfigPersister.h"

class ConfigPersisterNOP: public IConfigPersister {
public:
	void save (const std::map<std::string, ConfigVarPtr>& configVars) override
	{
	}

	std::string getValue (const std::string& name) const override
	{
		if (name == "persister")
			return "nop";
		return "";
	}

	void getVars (std::vector<std::string>& vars) const override
	{
	}
};
