#include "ConfigVar.h"
#include "common/Logger.h"

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
		error(LOG_CONFIG, _name + " is write protected");
		return;
	}
	_dirty = _value != value;
	_value = value;
	_intValue = string::toInt(_value);
	_floatValue = string::toFloat(_value);
	info(LOG_CONFIG, _name + " => changed value to: " + _value);
}
