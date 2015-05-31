#include "SDLSoundEngine.h"
#include "common/FileSystem.h"
#include "common/EventHandler.h"
#include "common/Logger.h"
#include "common/Math.h"
#include "common/SoundType.h"
#include "common/ExecutionTime.h"
#include "common/Singleton.h"
#include "GameRegistry.h"
#include "client/sound/Sound.h"
#include <SDL.h>
#include <SDL_mixer.h>
#include <assert.h>

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
	SDL_QuitSubSystem(SDL_INIT_AUDIO);
}

void SDLSoundEngine::channelFinished (int channel)
{
	assert(channel >= 0);
	assert(channel < lengthof(_channels));
	memset(&_channels[channel], 0, sizeof(_channels[channel]));
}

bool SDLSoundEngine::init (bool initCache)
{
	if (SDL_InitSubSystem(SDL_INIT_AUDIO) == -1) {
		error(LOG_CLIENT, String::format("unable to initialize audio: %s", SDL_GetError()));
		_state = SOUND_CLOSED;
		return false;
	}
	const int n = SDL_GetNumAudioDrivers();
	if (n == 0) {
		error(LOG_CLIENT, " no built-in audio drivers");
		_state = SOUND_CLOSED;
		SDL_QuitSubSystem(SDL_INIT_AUDIO);
		return false;
	} else {
		for (int i = 0; i < n; ++i) {
			info(LOG_CLIENT, String::format("available audio driver %s", SDL_GetAudioDriver(i)));
		}
	}

	info(LOG_CLIENT, String::format("actual audio driver: %s", SDL_GetCurrentAudioDriver()));

	const int audioRate = 44100;
	const Uint16 audioFormat = MIX_DEFAULT_FORMAT;
	const int audioChannels = MIX_DEFAULT_CHANNELS;
	const int audioBuffers = 4096;

	if (Mix_OpenAudio(audioRate, audioFormat, audioChannels, audioBuffers) != 0) {
		error(LOG_CLIENT, String::format("unable to initialize mixer: %s", Mix_GetError()));
		_state = false;
		SDL_QuitSubSystem(SDL_INIT_AUDIO);
		return false;
	}

	Mix_AllocateChannels(MAX_CHANNELS);
	Mix_ChannelFinished(channelFinished);

	info(LOG_CLIENT, "sound initialized");

	if (initCache) {
		ExecutionTime timeCache("Sound cache");
		for (SoundType::TypeMapConstIter i = SoundType::begin(); i != SoundType::end(); ++i) {
			SoundControl.cache(i->second->getSound());
		}
	}
	_state = SOUND_INITIALIZED;
	return true;
}

bool SDLSoundEngine::exists (const std::string& sound) const
{
	return FS.exists(FS.getSoundsDir() + sound + SOUNDTYPE);
}

void SDLSoundEngine::close ()
{
	info(LOG_CLIENT, "closing the sound system");
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
		error(LOG_CLIENT, "no music file to play was provided");
		return -1;
	}
	if (_musicPlaying == music)
		return -1;

	Mix_HaltMusic();
	Mix_FreeMusic(_music);
	_music = nullptr;

	SDL_RWops *rwops = FS.createRWops(FS.getDataDir() + FS.getSoundsDir() + music + SOUNDTYPE);
	if (rwops == nullptr) {
		error(LOG_CLIENT, "unable to load music file: " + music);
		return -1;
	}
#if SDL_MIXER_MAJOR_VERSION >= 2
	_music = Mix_LoadMUS_RW(rwops, 1);
#else
	_music = Mix_LoadMUS_RW(rwops);
#endif
	if (_music == nullptr) {
		error(LOG_CLIENT, String::format("unable to load music file: %s", Mix_GetError()));
		return -1;
	}

	const int ret = Mix_PlayMusic(_music, loop ? -1 : 1);
	if (ret == -1)
		error(LOG_CLIENT, String::format("unable to play music file: %s", Mix_GetError()));
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
		error(LOG_CLIENT, "no sound file to play was provided");
		return -1;
	}
	Mix_Chunk *sound = getChunk(filename);
	if (!sound)
		return -1;

	_currentChannel = (_currentChannel + 1) % MAX_CHANNELS;
	const int channel = Mix_PlayChannel(_currentChannel, sound, loop ? -1 : 0);
	if (channel == -1) {
		error(LOG_CLIENT, String::format("unable to play sound file: %s", Mix_GetError()));
	} else {
		assert(channel >= 0);
		assert(channel < lengthof(_channels));
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
	debug(LOG_CLIENT, "cache sound " + sound);
	return getChunk(sound) != nullptr;
}

Mix_Chunk* SDLSoundEngine::getChunk (const std::string& filename)
{
	if (filename.empty()) {
		error(LOG_CLIENT, "no sound file to get the chunk for was provided");
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
		error(LOG_CLIENT, String::format("unable to load sound file %s: %s", filename.c_str(), Mix_GetError()));
	}

	return sound;
}

void SDLSoundEngine::update (uint32_t deltaTime)
{
	_time += deltaTime;
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
		info(LOG_CLIENT, "sound is already paused");
		return;
	}
	info(LOG_CLIENT, "sound is now paused");
	_state |= SOUND_PAUSE;
	Mix_PauseMusic();
	Mix_Pause(-1);
}

void SDLSoundEngine::resume ()
{
	if (isActive()) {
		info(LOG_CLIENT, "sound is already active");
		return;
	}
	info(LOG_CLIENT, "sound is active again");
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
