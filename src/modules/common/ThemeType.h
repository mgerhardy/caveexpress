#pragma once

#include "common/Enum.h"

class ThemeType: public Enum<ThemeType> {
public:
	explicit ThemeType (const std::string& _name) :
		Enum<ThemeType>(_name)
	{
	}
};

namespace ThemeTypes {
extern ThemeType ICE;
extern ThemeType ROCK;
extern ThemeType JUNGLE;
extern ThemeType DESERT;

inline bool isIce (const ThemeType& other)
{
	return other == ICE;
}

inline bool isRock (const ThemeType& other)
{
	return other == ROCK;
}

inline bool isJungle (const ThemeType& other)
{
	return other == JUNGLE;
}

inline bool isDesert (const ThemeType& other)
{
	return other == DESERT;
}

}
