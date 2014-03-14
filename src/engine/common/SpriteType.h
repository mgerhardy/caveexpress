#pragma once

#include "engine/common/Enum.h"

class SpriteType: public Enum<SpriteType> {
public:
	SpriteType (const std::string& _name) :
			Enum<SpriteType>(_name)
	{
	}
};
