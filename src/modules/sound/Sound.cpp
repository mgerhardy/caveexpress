#include "Sound.h"
#include "SDLSoundEngine.h"
#include "common/FileSystem.h"
#include "common/Log.h"
#include "common/ConfigManager.h"
#include "common/ExecutionTime.h"
#include <stdio.h>
#include <stdlib.h>

Sound::Sound () :
		_soundEngine(&_dummy)
{
	const std::string& engine = Config.getSoundEngine();
	Log::info(LOG_SOUND, "soundengine: %s", engine.c_str());
	if (engine == "sdl")
		_soundEngine = new SDLSoundEngine();
	else
		Log::info(LOG_SOUND, "disable sound");
}

Sound::~Sound ()
{
	close();
}

Sound& Sound::get ()
{
	static Sound _sound;
	return _sound;
}

void Sound::close ()
{
	Log::info(LOG_SOUND, "shutting down the sound engine");
	_soundEngine->close();
	if (_soundEngine != &_dummy) {
		delete _soundEngine;
		_soundEngine = &_dummy;
	}
}
