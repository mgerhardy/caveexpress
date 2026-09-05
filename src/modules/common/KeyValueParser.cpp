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
			Log::error(LOG_COMMON, "invalid key value setting found: %s", str.c_str());
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

void KeyValueParser::set (const std::string& key, const std::string& value)
{
	if (value.empty())
		_settings.erase(key);
	else
		_settings[key] = value;
}

void KeyValueParser::set (const std::string& key, float value)
{
	set(key, string::toString(value));
}

void KeyValueParser::set (const std::string& key, bool value)
{
	set(key, std::string(value ? "true" : "false"));
}

void KeyValueParser::remove (const std::string& key)
{
	_settings.erase(key);
}

std::string KeyValueParser::str () const
{
	std::string out;
	for (const auto& entry : _settings) {
		if (!out.empty())
			out += ",";
		out += entry.first;
		out += "=";
		out += entry.second;
	}
	return out;
}
