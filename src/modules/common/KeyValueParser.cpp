#include "common/KeyValueParser.h"
#include "common/Log.h"

KeyValueParser::KeyValueParser (const std::string& settings)
{
	std::vector<std::string> keyValues;
	string::splitString(settings, keyValues, ",");
	for (std::vector<std::string>::const_iterator i = keyValues.begin(); i != keyValues.end(); ++i) {
		const std::string& str = *i;
		std::vector<std::string> keyValue;
		string::splitString(str, keyValue, "=");
		if (keyValue.size() != 2) {
			Log::error(LOG_GENERAL, "invalid key value setting found: %s", str.c_str());
			continue;
		}
		_settings[keyValue[0]] = keyValue[1];
	}
}

const std::string KeyValueParser::getString (const std::string& key, const std::string& defaultVal) const
{
	SettingsMap::const_iterator i = _settings.find(key);
	if (i == _settings.end()) {
		return defaultVal;
	}
	return i->second;
}
