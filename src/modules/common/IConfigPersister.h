#pragma once

#include "common/Compiler.h"
#include "common/ConfigVar.h"
#include <map>
#include <string>
#include <vector>

class IConfigPersister {
public:
	virtual ~IConfigPersister ()
	{
	}

	virtual void init () {}
	virtual void save (const std::map<std::string, ConfigVarPtr>& configVars) = 0;
	virtual std::string getValue (const std::string& name) const = 0;
	virtual void getVars (std::vector<std::string>& vars) const = 0;
};
