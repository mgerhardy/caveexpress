#pragma once

#include "common/String.h"
#include "common/Log.h"
#include <map>
#include <cmath>

typedef float gridCoord;
typedef float gridSize;

class IMap {
public:
	struct StartPosition {std::string _x; std::string _y;};
	typedef std::vector<StartPosition> StartPositions;
	typedef StartPositions::const_iterator StartPositionsConstIter;
	typedef std::map<std::string, std::string> SettingsMap;
	typedef SettingsMap::const_iterator SettingsMapConstIter;

protected:
	SettingsMap _settings;

	StartPositions _startPositions;

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

	inline bool getStartPosition(int index, int&x, int& y) const
	{
		const int size = _startPositions.size();
		if (size == 1) {
			index = 0;
		}
		if (index < 0 || index > size) {
			x = -1;
			y = -1;
			return false;
		}
		const IMap::StartPosition& pos = _startPositions[index];
		x = string::toInt(pos._x);
		y = string::toInt(pos._y);
		return true;
	}

	inline bool getStartPosition(int index, float&x, float& y) const
	{
		const int size = _startPositions.size();
		if (size == 1) {
			index = 0;
		}
		if (index < 0 || index >= size) {
			x = -1.0f;
			y = -1.0f;
			return false;
		}
		const IMap::StartPosition& pos = _startPositions[index];
		x = string::toFloat(pos._x);
		y = string::toFloat(pos._y);
		return true;
	}

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
		Log::debug(LOG_MAP, "key: %s = %s", key.c_str(), val.c_str());
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

