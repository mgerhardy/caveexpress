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

	virtual bool init (bool initCache) override {return false;}
	virtual void close () override {}
	virtual bool exists (const std::string& sound) const override { return false; }
	virtual void halt (int sound) override {}
	virtual bool cache (const std::string& sound) override { return false; }
	virtual void haltAll () override {}
	virtual void pause () override {}
	virtual void resume () override {}
	virtual int playMusic (const std::string& music, bool loop) override { return -1; }
	virtual void haltMusic (int music) override {}
	virtual int play (const std::string& filename, const vec2& position, bool loop) override { return -1; }
	virtual void update (uint32_t deltaTime) override {}
	virtual void setListenerPosition (const vec2& position, const vec2& velocity = vec2_zero) override {}
};
