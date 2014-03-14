#pragma once

#include "engine/common/Math.h"
#include <string>

class ISoundEngine {
public:
	ISoundEngine ()
	{
	}
	virtual ~ISoundEngine ()
	{
	}

	virtual bool init (bool initCache) = 0;
	virtual void close () = 0;
	virtual bool exists (const std::string& sound) const = 0;
	virtual bool cache (const std::string& sound) = 0;
	virtual void halt (int sound) = 0;
	virtual void haltAll () = 0;
	virtual void pause () = 0;
	virtual void resume () = 0;
	virtual int playMusic (const std::string& music, bool loop) = 0;
	virtual void haltMusic (int music) = 0;
	virtual int play (const std::string& filename, const vec2& position, bool loop) = 0;
	virtual void update (uint32_t deltaTime) = 0;
	virtual void setListenerPosition (const vec2& position, const vec2& velocity = vec2_zero) = 0;
};
