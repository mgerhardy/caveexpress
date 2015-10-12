#pragma once

#include "common/Singleton.h"
#include <string>

class Application {
protected:
	std::string _organisation;
	std::string _name;
	// stuff like org.cavepacker
	std::string _id;
	std::string _version;
public:
	Application() :
			_organisation("caveproductions"), _version("0.0") {
	}

	inline void setOrganisation(const std::string& organisation) {
		_organisation = organisation;
	}

	inline void setId(const std::string& id) {
		_id = id;
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

	inline const std::string& getId() {
		return _id;
	}
};
