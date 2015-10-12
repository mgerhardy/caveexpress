#pragma once

#include "common/Singleton.h"
#include <string>

class Application {
protected:
	std::string _organisation;
	std::string _name;
	std::string _version;
	std::string _packageName;
public:
	Application() :
			_organisation("caveproductions") {
	}

	inline void setPackageName(const std::string& packageName) {
		_packageName = packageName;
	}

	inline const std::string& getPackageName() const {
		return _packageName;
	}

	inline void setOrganisation(const std::string& organisation) {
		_organisation = organisation;
	}

	inline void setName(const std::string& name) {
		_name = name;
	}

	inline void setVersion(const std::string& version) {
		_version = version;
	}

	inline const std::string& getVersion() {
		return _version;
	}

	inline const std::string& getName() {
		return _name;
	}

	inline const std::string& getOrganisation() {
		return _organisation;
	}
};
