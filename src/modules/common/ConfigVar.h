#pragma once

#include <memory>
#include "common/String.h"
#include <string>

const unsigned int CV_NOPERSIST = 1 << 0;
const unsigned int CV_READONLY = (1 << 1) | CV_NOPERSIST;

class ConfigVar {
private:
	const std::string _name;
	const unsigned int _flags;

	float _floatValue;
	int _intValue;
	std::string _value;
	bool _dirty;

public:
	ConfigVar (const std::string& name, const std::string& value = "", unsigned int flags = 0u);
	virtual ~ConfigVar ();

	int getIntValue () const;
	float getFloatValue () const;
	bool getBoolValue () const;
	unsigned int getFlags () const;
	void setValue (int value) { setValue(string::toString(value)); }
	void setValue (float value) { setValue(string::toString(value)); }
	void setValue (const std::string& value);
	const std::string& getValue () const;
	const std::string& getName () const;
	bool isDirty () const;
	void resetDirtyState ();
};

inline float ConfigVar::getFloatValue () const
{
	return _floatValue;
}

inline int ConfigVar::getIntValue () const
{
	return _intValue;
}

inline bool ConfigVar::getBoolValue () const
{
	return _value == "true" || _value == "1";
}

inline const std::string& ConfigVar::getValue () const
{
	return _value;
}

inline const std::string& ConfigVar::getName () const
{
	return _name;
}

inline void ConfigVar::resetDirtyState ()
{
	_dirty = false;
}

inline bool ConfigVar::isDirty () const
{
	return _dirty;
}

inline unsigned int ConfigVar::getFlags () const
{
	return _flags;
}

typedef std::shared_ptr<ConfigVar> ConfigVarPtr;
