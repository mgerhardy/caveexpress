#include "ConfigVar.h"
#include "common/Log.h"

ConfigVar::ConfigVar (const std::string& name, const std::string& value, unsigned int flags) :
		_name(name), _flags(flags), _value(value), _dirty(false)
{
	_intValue = string::toInt(_value);
	_floatValue = string::toFloat(_value);
}

ConfigVar::~ConfigVar ()
{
}

void ConfigVar::setValue (const std::string& value)
{
	if (_flags & CV_READONLY) {
		Log::error(LOG_CONFIG, "%s is write protected", _name.c_str());
		return;
	}
	_dirty = _value != value;
	_value = value;
	_intValue = string::toInt(_value);
	_floatValue = string::toFloat(_value);
	Log::info(LOG_CONFIG, "%s => changed value to: %s", _name.c_str(), _value.c_str());
}
