#pragma once

#include "engine/common/Enum.h"
#include "engine/common/String.h"

class Animation: public Enum<Animation> {
protected:
	const bool _hasDirection;
	std::string _nameWithoutDirection;
public:
	const bool loop;

	Animation (const std::string& _name, bool _loop = true) :
			Enum<Animation>(_name), loop(_loop), _hasDirection(string::endsWith(_name, "-left") || string::endsWith(_name, "-right"))
	{
		if (!_hasDirection)
			_nameWithoutDirection = name;
		else if (string::endsWith(name, "-left"))
			_nameWithoutDirection = name.substr(0, name.length() - 5);
		else
			_nameWithoutDirection = name.substr(0, name.length() - 6);
	}

	const std::string& getNameWithoutDirection () const
	{
		return _nameWithoutDirection;
	}

	inline bool hasDirection () const
	{
		return _hasDirection;
	}
};
