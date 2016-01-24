#pragma once

#include "common/Math.h"
#include <string>

class ISoundEngine {
public:
	ISoundEngine ()
	{
	}
	virtual ~ISoundEngine ()
	{
	}

	virtual bool init (bool initCache) {return false;}
	virtual void close () {}
	virtual bool exists (const std::string& sound) const { return false; }
	virtual void halt (int sound) {}
	virtual bool cache (const std::string& sound) { return false; }
	virtual void haltAll () {}
	virtual int volume (int newVolume) { return newVolume; }
	virtual int musicVolume (int newVolume) { return newVolume; }
	virtual void pause () {}
	virtual void resume () {}
	virtual int playMusic (const std::string& music, bool loop) { return -1; }
	virtual void haltMusic (int music) {}
	virtual int play (const std::string& filename, const vec2& position, bool loop) { return -1; }
	virtual void update (uint32_t deltaTime) {}
	virtual void setListenerPosition (const vec2& position, const vec2& velocity = vec2_zero) {}
};
