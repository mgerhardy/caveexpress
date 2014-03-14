#pragma once

#include "engine/common/String.h"
#include "engine/common/Logger.h"
#include <map>
#include <cmath>

typedef float gridCoord;
typedef float gridSize;

class IMap {
public:
	typedef std::map<std::string, std::string> SettingsMap;
	typedef SettingsMap::const_iterator SettingsMapConstIter;

protected:
	SettingsMap _settings;

	// the name of the map
	std::string _name;
	std::string _title;

public:
	IMap ()
	{
	}
	virtual ~IMap ()
	{
	}

	/**
	 * @param[in] deltaTime The milliseconds since the last frame was executed
	 */
	virtual void update (uint32_t deltaTime) = 0;

	virtual bool isActive () const = 0;

	// delay is given in millis
	virtual void restart (uint32_t delay) = 0;

	virtual int getMapWidth () const = 0;
	virtual int getMapHeight () const = 0;

	// will return a setting for a map from the map definition.
	inline String getSetting (const std::string& key, const std::string& defaultValue = "") const
	{
		std::string val;
		SettingsMapConstIter iter = _settings.find(key);
		if (iter != _settings.end()) {
			val = iter->second;
		}

		if (val.empty())
			val = defaultValue;
		debug(LOG_MAP, "key: " + key + " = " + val);
		return val;
	}

	inline const SettingsMap& getSettings () const
	{
		return _settings;
	}

	inline const std::string& getName () const
	{
		return _name;
	}

	inline const std::string& getTitle () const
	{
		return _title;
	}
};

