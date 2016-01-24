#include "SDLSoundEngine.h"
#include "common/FileSystem.h"
#include "common/EventHandler.h"
#include "common/Log.h"
#include "common/Math.h"
#include "common/SoundType.h"
#include "common/ExecutionTime.h"
#include "common/Singleton.h"
#include <SDL.h>
#include <SDL_assert.h>
#include <SDL_mixer.h>

namespace {
const int SOUND_CLOSED = 1 << 0;
const int SOUND_INITIALIZED = 1 << 1;
const int SOUND_PAUSE = 1 << 2;
}

#define SOUNDTYPE ".ogg"

SDLSoundEngine::Channel SDLSoundEngine::_channels[MAX_CHANNELS];

SDLSoundEngine::SDLSoundEngine () :
		_music(nullptr), _currentChannel(0), _state(SOUND_CLOSED), _time(0)
{
	_listenerPosition = vec2_zero;
	_volume = Config.getConfigVar("volume", string::toString(MIX_MAX_VOLUME));
	_musicVolume = Config.getConfigVar("musicvolume", string::toString(MIX_MAX_VOLUME));
}

SDLSoundEngine::~SDLSoundEngine ()
{
	Mix_HaltMusic();
	Mix_FreeMusic(_music);
	Mix_AllocateChannels(0);
	for (ChunkMapIter i = _map.begin(); i != _map.end(); ++i) {
		Mix_FreeChunk(i->second);
	}
	_map.clear();
	Mix_CloseAudio();
	Mix_Quit();
	SDL_QuitSubSystem(SDL_INIT_AUDIO);
}

void SDLSoundEngine::channelFinished (int channel)
{
	SDL_assert(channel >= 0);
	SDL_assert(channel < lengthofi(_channels));
	memset(&_channels[channel], 0, sizeof(_channels[channel]));
}

bool SDLSoundEngine::init (bool initCache)
{
	if (SDL_InitSubSystem(SDL_INIT_AUDIO) == -1) {
		Log::error(LOG_SOUND, "unable to initialize audio: %s", SDL_GetError());
		_state = SOUND_CLOSED;
		return false;
	}
	const int n = SDL_GetNumAudioDrivers();
	if (n == 0) {
		Log::error(LOG_SOUND, " no built-in audio drivers");
		_state = SOUND_CLOSED;
		SDL_QuitSubSystem(SDL_INIT_AUDIO);
		return false;
	} else {
		for (int i = 0; i < n; ++i) {
			Log::info(LOG_SOUND, "available audio driver %s", SDL_GetAudioDriver(i));
		}
	}

	const int result = Mix_Init(MIX_INIT_OGG);
	if (!(result &~ MIX_INIT_OGG)) {
		Log::error(LOG_SOUND, "Failed to initialize sdl mixer with ogg support");
	}

	Log::info(LOG_SOUND, "audio driver: %s", SDL_GetCurrentAudioDriver());

	const int audioRate = 44100;
	const Uint16 audioFormat = MIX_DEFAULT_FORMAT;
	const int audioChannels = MIX_DEFAULT_CHANNELS;
	const int audioBuffers = 4096;

	if (Mix_OpenAudio(audioRate, audioFormat, audioChannels, audioBuffers) != 0) {
		Log::error(LOG_SOUND, "unable to initialize mixer: %s", Mix_GetError());
		_state = false;
		SDL_QuitSubSystem(SDL_INIT_AUDIO);
		return false;
	}

	Mix_AllocateChannels(MAX_CHANNELS);
	Mix_ChannelFinished(channelFinished);

	Log::info(LOG_SOUND, "sound initialized");

	if (initCache) {
		ExecutionTime timeCache("Sound cache");
		for (SoundType::TypeMapConstIter i = SoundType::begin(); i != SoundType::end(); ++i) {
			cache(i->second->getSound());
		}
	}
	_state = SOUND_INITIALIZED;
	Mix_Volume(-1, _volume->getIntValue());
	Mix_VolumeMusic(_musicVolume->getIntValue());
	return true;
}

bool SDLSoundEngine::exists (const std::string& sound) const
{
	return FS.exists(FS.getSoundsDir() + sound + SOUNDTYPE);
}

void SDLSoundEngine::close ()
{
	Log::info(LOG_SOUND, "closing the sound system");
	Mix_HaltMusic();
	Mix_FreeMusic(_music);
	_music = nullptr;
	_musicPlaying = "";

	haltAll();
}

void SDLSoundEngine::halt (int sound)
{
	if (sound == -1)
		return;

	Mix_HaltChannel(sound);
}

void SDLSoundEngine::haltAll ()
{
	Mix_HaltChannel(-1);
}

int SDLSoundEngine::playMusic (const std::string& music, bool loop)
{
	if (!isActive()) {
		return -1;
	}
	if (music.empty()) {
		Log::error(LOG_SOUND, "no music file to play was provided");
		return -1;
	}
	if (_musicPlaying == music)
		return -1;

	Mix_HaltMusic();
	Mix_FreeMusic(_music);
	_music = nullptr;

	SDL_RWops *rwops = FS.createRWops(FS.getDataDir() + FS.getSoundsDir() + music + SOUNDTYPE);
	if (rwops == nullptr) {
		Log::error(LOG_SOUND, "unable to load music file: %s", music.c_str());
		return -1;
	}
#if SDL_MIXER_MAJOR_VERSION >= 2
	_music = Mix_LoadMUS_RW(rwops, 1);
#else
	_music = Mix_LoadMUS_RW(rwops);
#endif
	if (_music == nullptr) {
		Log::error(LOG_SOUND, "unable to load music file: %s", Mix_GetError());
		return -1;
	}

	const int ret = Mix_PlayMusic(_music, loop ? -1 : 1);
	if (ret == -1)
		Log::error(LOG_SOUND, "unable to play music file: %s", Mix_GetError());
	else
		_musicPlaying = music;
	return ret;
}

void SDLSoundEngine::haltMusic (int music)
{
	if (!isActive()) {
		return;
	}

	if (music == -1)
		return;

	Mix_HaltMusic();
	Mix_FreeMusic(_music);
	_music = nullptr;
	_musicPlaying = "";
}

int SDLSoundEngine::play (const std::string& filename, const vec2& position, bool loop)
{
	if (!isActive()) {
		return -1;
	}
	if (filename.empty()) {
		Log::error(LOG_SOUND, "no sound file to play was provided");
		return -1;
	}
	Mix_Chunk *sound = getChunk(filename);
	if (!sound)
		return -1;

	_currentChannel = (_currentChannel + 1) % MAX_CHANNELS;
	const int channel = Mix_PlayChannel(_currentChannel, sound, loop ? -1 : 0);
	if (channel == -1) {
		Log::error(LOG_SOUND, "unable to play sound file: %s", Mix_GetError());
	} else {
		SDL_assert(channel >= 0);
		SDL_assert(channel < lengthofi(_channels));
		_channels[channel].channel = channel;
		_channels[channel].chunk = sound;
		_channels[channel].pos = position;
	}
	return channel;
}

bool SDLSoundEngine::cache (const std::string& sound)
{
	if (!isActive()) {
		return false;
	}
	Log::debug(LOG_SOUND, "cache sound %s", sound.c_str());
	return getChunk(sound) != nullptr;
}

Mix_Chunk* SDLSoundEngine::getChunk (const std::string& filename)
{
	if (filename.empty()) {
		Log::error(LOG_SOUND, "no sound file to get the chunk for was provided");
		return nullptr;
	}
	ChunkMapIter i = _map.find(filename);
	if (i != _map.end()) {
		return i->second;
	}

	const std::string& path = FS.getDataDir() + FS.getSoundsDir() + filename + SOUNDTYPE;
	SDL_RWops *rwops = FS.createRWops(path);
	Mix_Chunk *sound = Mix_LoadWAV_RW(rwops, 1);
	_map[filename] = sound;
	if (sound == nullptr) {
		Log::error(LOG_SOUND, "unable to load sound file %s: %s", filename.c_str(), Mix_GetError());
	}

	return sound;
}

void SDLSoundEngine::update (uint32_t deltaTime)
{
	_time += deltaTime;

	if (_volume->isDirty()) {
		_volume->resetDirtyState();
		volume(_volume->getIntValue());
	}

	if (_musicVolume->isDirty()) {
		_musicVolume->resetDirtyState();
		musicVolume(_musicVolume->getIntValue());
	}

#ifndef EMSCRIPTEN
	if (!isActive()) {
		return;
	}
	const double scale = 60.0;
	for (int i = 0; i < MAX_CHANNELS; ++i) {
		Channel &channel = _channels[i];
		if (channel.pos.isZero())
			continue;
		const double xDiff = (_listenerPosition.x - channel.pos.x) * scale;
		const double yDiff = (_listenerPosition.y - channel.pos.y) * scale;
		const int dist = std::sqrt(xDiff * xDiff + yDiff * yDiff);
		const double angleInDegrees = getAngleBetweenPoints(_listenerPosition.x, _listenerPosition.y, channel.pos.x,
				channel.pos.y);
		Mix_SetPosition(channel.channel, static_cast<int16_t>(angleInDegrees), std::min(dist, 255));
	}
#endif
}

void SDLSoundEngine::setListenerPosition (const vec2& position, const vec2& velocity)
{
	_listenerPosition = position;
}

void SDLSoundEngine::pause ()
{
	if (!isActive()) {
		Log::info(LOG_SOUND, "sound is already paused");
		return;
	}
	Log::info(LOG_SOUND, "sound is now paused");
	_state |= SOUND_PAUSE;
	Mix_PauseMusic();
	Mix_Pause(-1);
}

void SDLSoundEngine::resume ()
{
	if (isActive()) {
		Log::info(LOG_SOUND, "sound is already active");
		return;
	}
	Log::info(LOG_SOUND, "sound is active again");
	_state &= ~SOUND_PAUSE;
	Mix_ResumeMusic();
	Mix_Resume(-1);
}

bool SDLSoundEngine::isActive () const
{
	if (!(_state & SOUND_INITIALIZED)) {
		return false;
	}
	if (_state & SOUND_PAUSE) {
		return false;
	}
	return true;
}

int SDLSoundEngine::volume (int newVolume)
{
	return Mix_Volume(-1, newVolume);
}

int SDLSoundEngine::musicVolume (int newVolume)
{
	return Mix_VolumeMusic(newVolume);
}
