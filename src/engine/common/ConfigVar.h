#pragma once

#include "engine/common/Pointers.h"
#include "engine/common/String.h"
#include <string>

const unsigned int CV_READONLY = 1 << 0;

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
	void setValue (const std::string& value);
	const std::string& getValue () const;
	const std::string& getName () const;
	bool isDirty () const;
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

inline bool ConfigVar::isDirty () const
{
	return _dirty;
}

typedef SharedPtr<ConfigVar> ConfigVarPtr;
