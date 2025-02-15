#pragma once

#include "common/ConfigManager.h"
#include "ISoundEngine.h"
#include <unordered_map>

struct Mix_Chunk;
struct Mix_Music;

#define MAX_CHANNELS 16

class SDLSoundEngine: public ISoundEngine {
private:
	typedef std::unordered_map<std::string, Mix_Chunk*> ChunkMap;
	typedef ChunkMap::iterator ChunkMapIter;
	ChunkMap _map;
	vec2 _listenerPosition;
	Mix_Music *_music;
	std::string _musicPlaying;
	int _currentChannel;
	int _state;
	uint32_t _time;
	ConfigVarPtr _volume;
	ConfigVarPtr _musicVolume;

	Mix_Chunk* getChunk (const std::string& filename);

	struct Channel {
		int channel = 0;
		Mix_Chunk *chunk = nullptr;
		vec2 pos;
	};

	static Channel _channels[MAX_CHANNELS];
	static void channelFinished (int channel);

	inline bool isActive () const;

public:
	SDLSoundEngine ();
	virtual ~SDLSoundEngine ();

	// ISoundEngine
	bool init (bool initCache) override;
	void close () override;
	bool exists (const std::string& sound) const override;
	int playMusic (const std::string& music, bool loop) override;
	void haltMusic (int music) override;
	bool cache (const std::string& sound) override;
	void halt (int sound) override;
	void haltAll () override;
	void pause () override;
	void resume () override;
	int play (const std::string& filename, const vec2& position, bool loop) override;
	void update (uint32_t deltaTime) override;
	void setListenerPosition (const vec2& position, const vec2& velocity) override;
	int volume (int newVolume) override;
	int musicVolume (int newVolume) override;
};
