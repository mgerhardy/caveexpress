#pragma once

#include "common/Enum.h"
#include "common/String.h"
#include <string>
#include <stdlib.h>

class SoundType : public Enum<SoundType> {
private:
	bool _loop;
	uint8_t _randomSounds;
public:
	SoundType (const std::string& sound, bool loop = false, uint8_t randomSounds = 0) :
			Enum<SoundType>(sound), _loop(loop), _randomSounds(randomSounds)
	{
	}

	inline bool isLoopSound () const
	{
		return _loop;
	}

	std::string getSound () const
	{
		if (_randomSounds == 0)
			return name;

		const int rnd = rand() % _randomSounds + 1;
		return name + string::toString(rnd);
	}
};
