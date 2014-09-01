#pragma once

enum SoundType {
	SOUND_BEGIN, SOUND_END
};

namespace sound {
inline bool isLoopSound (const SoundType type)
{
	return false;
}

inline std::string getSoundByType (const SoundType type)
{
	return "";
}
}
