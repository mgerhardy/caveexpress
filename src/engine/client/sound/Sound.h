#pragma once

#include "engine/common/NonCopyable.h"
#include "ISoundEngine.h"

class Sound: public NonCopyable {
private:
	Sound ();
	Sound (const Sound&);
	Sound& operator= (const Sound&);

	ISoundEngine *_soundEngine;
	ISoundEngine _dummy;

public:
	virtual ~Sound ();

	bool init (bool initCache = false);
	void close ();
	void pause ();
	void resume ();
	bool exists (const std::string& sound) const;
	void haltMusic (int music);
	int playMusic (const std::string& music, bool loop = true);
	void halt (int sound);
	void haltAll ();
	int play (const std::string& path, const vec2& position = vec2_zero, bool loop = false);
	void update (uint32_t deltaTime);
	bool cache (const std::string& sound);
	void setListenerPosition (const vec2& position, const vec2& velocity = vec2_zero);
	static Sound& get ();
};

inline bool Sound::init (bool initCache)
{
	return _soundEngine->init(initCache);
}

inline void Sound::update (uint32_t deltaTime)
{
	return _soundEngine->update(deltaTime);
}

inline bool Sound::cache (const std::string& sound)
{
	return _soundEngine->cache(sound);
}

inline bool Sound::exists (const std::string& sound) const
{
	return _soundEngine->exists(sound);
}

inline void Sound::haltMusic (int music)
{
	_soundEngine->haltMusic(music);
}

inline int Sound::playMusic (const std::string& music, bool loop)
{
	return _soundEngine->playMusic(music, loop);
}

inline void Sound::halt (int sound)
{
	_soundEngine->halt(sound);
}

inline void Sound::haltAll ()
{
	_soundEngine->haltAll();
}

inline int Sound::play (const std::string& path, const vec2& position, bool loop)
{
	return _soundEngine->play(path, position, loop);
}

inline void Sound::pause ()
{
	return _soundEngine->pause();
}

inline void Sound::resume ()
{
	return _soundEngine->resume();
}

inline void Sound::setListenerPosition (const vec2& position, const vec2& velocity)
{
	_soundEngine->setListenerPosition(position, velocity);
}

#define SoundControl Sound::get()
