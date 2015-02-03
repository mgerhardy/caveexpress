#include "Sound.h"
#include "engine/client/sound/sdl/SDLSoundEngine.h"
#include "engine/common/FileSystem.h"
#include "engine/common/Logger.h"
#include "engine/common/ConfigManager.h"
#include "engine/common/ExecutionTime.h"
#include <stdio.h>
#include <stdlib.h>

Sound::Sound () :
		_soundEngine(&_dummy)
{
	const String& engine = Config.getSoundEngine();
	info(LOG_CLIENT, "soundengine: " + engine);
	if (engine == "sdl")
		_soundEngine = new SDLSoundEngine();
	else
		info(LOG_CLIENT, "disable sound");
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
	info(LOG_CLIENT, "shutting down the sound engine");
	_soundEngine->close();
	if (_soundEngine != &_dummy) {
		delete _soundEngine;
		_soundEngine = &_dummy;
	}
}
