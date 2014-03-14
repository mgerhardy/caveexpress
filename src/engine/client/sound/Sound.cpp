#include "Sound.h"
#include "engine/client/sound/sdl/SDLSoundEngine.h"
#include "engine/client/sound/DummySoundEngine.h"
#include "engine/common/FileSystem.h"
#include "engine/common/Logger.h"
#include "engine/common/ConfigManager.h"
#include "engine/common/ExecutionTime.h"
#include <stdio.h>
#include <stdlib.h>

Sound::Sound ()
{
	const String& engine = Config.getSoundEngine();
	if (engine == "sdl")
		_soundEngine = new SDLSoundEngine();
	else
		_soundEngine = new DummySoundEngine();
}

Sound::~Sound ()
{
	info(LOG_CLIENT, "shutting down the sound engine");
	close();

	delete _soundEngine;
}

Sound& Sound::get ()
{
	static Sound _sound;
	return _sound;
}
