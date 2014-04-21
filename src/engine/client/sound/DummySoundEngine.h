#pragma once

#include "ISoundEngine.h"

class DummySoundEngine: public ISoundEngine {
public:
	DummySoundEngine () {}
	virtual ~DummySoundEngine () {}

	// ISoundEngine
	bool init (bool initCache) override {return false;}
	void close () override {}
	bool exists (const std::string& sound) const override { return false; }
	void halt (int sound) override {}
	bool cache (const std::string& sound) override { return false; }
	void haltAll () override {}
	void pause () override {}
	void resume () override {}
	int playMusic (const std::string& music, bool loop) override { return -1; }
	void haltMusic (int music) override {}
	int play (const std::string& filename, const vec2& position, bool loop) override { return -1; }
	void update (uint32_t deltaTime) override {}
	void setListenerPosition (const vec2& position, const vec2& velocity) override {}
};
