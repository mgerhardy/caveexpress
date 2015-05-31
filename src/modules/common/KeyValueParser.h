#pragma once

#include "common/String.h"
#include <string>
#include <vector>
#include <map>

class KeyValueParser {
protected:
	typedef std::map<std::string, std::string> SettingsMap;
	SettingsMap _settings;
public:
	KeyValueParser (const std::string& settings);

	inline float getFloat (const std::string& key, float defaultVal = 0.0f) const
	{
		return string::toFloat(getString(key, string::toString(defaultVal)));
	}

	inline int getInt (const std::string& key, int defaultVal = 0) const
	{
		return string::toInt(getString(key, string::toString(defaultVal)));
	}

	inline bool getBool (const std::string& key, bool defaultVal = false) const
	{
		return string::toBool(getString(key, defaultVal ? "true" : "false"));
	}

	const std::string getString (const std::string& key, const std::string& defaultVal = "") const;
};
