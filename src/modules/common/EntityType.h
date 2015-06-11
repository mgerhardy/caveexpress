#pragma once

#include "common/ThemeType.h"
#include "common/Math.h"
#include "common/Enum.h"
#include "common/String.h"

#define DEFAULT_LIGHT_STATE false

enum {
	CLIENT_OFFSET_TOP,
	CLIENT_OFFSET_RIGHT,
	CLIENT_OFFSET_BOTTOM,
	CLIENT_OFFSET_LEFT,
	CLIENT_OFFSET_MAX
};

class EntityType: public Enum<EntityType> {
protected:
	mutable std::string _nameWithoutTheme;
	mutable bool _hasTheme;
public:
	float width;
	float height;

	EntityType (const std::string& _name) :
			Enum<EntityType>(_name), _nameWithoutTheme(""), _hasTheme(false), width(1.0f), height(1.0f)
	{
	}

	inline void setSize (float _width, float _height)
	{
		if (_width > 0.0f)
			width = _width;
		if (_height > 0.0f)
			height = _height;
	}

	inline const std::string& getNameWithoutTheme () const
	{
		initThemeData();
		return _nameWithoutTheme;
	}

	inline bool hasTheme () const
	{
		initThemeData();
		return _hasTheme;
	}

	inline void initThemeData () const
	{
		if (_hasTheme)
			return;

		if (this->isNone())
			return;

		if (!_nameWithoutTheme.empty())
			return;

		if (string::endsWith(name, ThemeTypes::ICE.name)) {
			_nameWithoutTheme = name.substr(0, name.length() - ThemeTypes::ICE.name.length() - 1);
			_hasTheme = true;
		} else if (string::endsWith(name, ThemeTypes::ROCK.name)) {
			_nameWithoutTheme = name.substr(0, name.length() - ThemeTypes::ROCK.name.length() - 1);
			_hasTheme = true;
		} else {
			_nameWithoutTheme = name;
			_hasTheme = false;
		}
	}
};
