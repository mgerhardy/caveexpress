#pragma once

#include "engine/client/textures/Texture.h"
#include "engine/common/Layer.h"
#include "engine/common/Pointers.h"
#include "engine/common/IFrontend.h"
#include <vector>
#include <string>
#include <stdint.h>

class Sprite {
private:
	typedef std::vector<TexturePtr> AnimationFrames;
	typedef AnimationFrames::iterator AnimationFramesIterator;

	Sprite (const std::string& name, const std::vector<int>& delays, const std::vector<bool>& active,
			const AnimationFrames* textures, int frameCount, float fps, int32_t frameTimeRemaining, bool loop,
			int spriteWidth, int spriteHeight);

	std::vector<int> _delays;
	std::vector<bool> _active;

	std::string _name;
	AnimationFrames _textures[MAX_LAYERS];
	int _currentFrame;
	int _frameCount;
	float _fps;
	int32_t _frameTimeRemaining;
	bool _loop;
	int _spriteWidth;
	int _spriteHeight;

public:
	Sprite (const std::string& name);
	virtual ~Sprite ();

	TexturePtr getActiveTexture (Layer layer) const;
	void setFPS (float fps);
	void setTile (bool tile);

	void setDelay (int frame, int delay);
	void setActive (int frame, bool active);
	// this frame can trigger an entity state change depending on the value of this boolean
	bool isActive (int frame) const;
	bool isLoop () const;
	void setLoop (bool loop);
	bool addFrame (Layer layer, const std::string& name, int delay = 0, bool active = true);
	void update (uint32_t deltaTime);

	int getFrameCount () const;
	int getWidth (Layer layer) const;
	int getHeight (Layer layer) const;
	int getMaxWidth () const;
	int getMaxHeight () const;

	// render the sprite fullscreen but at a given y level
	// param[in] y normalized y screen coordinates (0 - EventHandler::HEIGHT)
	bool render (IFrontend *frontend, Layer layer, int y) const;
	// param[in] x normalized x screen coordinates (0 - EventHandler::WIDTH)
	// param[in] y normalized y screen coordinates (0 - EventHandler::HEIGHT)
	bool render (IFrontend *frontend, Layer layer, int x, int y, int w, int h, int16_t angle = 0, float alpha = 1.0f) const;
	// param[in] x normalized x screen coordinates (0 - EventHandler::WIDTH)
	// param[in] y normalized y screen coordinates (0 - EventHandler::HEIGHT)
	bool render (IFrontend *frontend, Layer layer, int x, int y, int16_t angle = 0, float alpha = 1.0f) const;

	void setCurrentFrame (int frame);

	const std::string& getName () const;

	Sprite* copy () const;
};

inline const std::string& Sprite::getName () const
{
	return _name;
}

inline TexturePtr Sprite::getActiveTexture (Layer layer) const
{
	const AnimationFrames& frames = _textures[layer];
	const size_t size = frames.size();
	if (size == 0)
		return TexturePtr();
	if (size < _currentFrame)
		return _textures[layer].at(size - 1);
	return _textures[layer].at(_currentFrame);
}

inline int Sprite::getWidth (Layer layer) const
{
	if (_currentFrame == -1)
		return 0;
	const TexturePtr t = getActiveTexture(layer);
	if (!t) {
		return 0;
	}
	return t->getWidth();
}

inline int Sprite::getMaxWidth () const
{
	int w = 0;
	for (Layer layer = LAYER_BACK; layer < MAX_LAYERS; ++layer) {
		const TexturePtr t = getActiveTexture(layer);
		if (!t)
			continue;
		w = std::max(w, t->getWidth());
	}
	return w;
}

inline int Sprite::getMaxHeight () const
{
	int h = 0;
	for (Layer layer = LAYER_BACK; layer < MAX_LAYERS; ++layer) {
		const TexturePtr t = getActiveTexture(layer);
		if (!t)
			continue;
		h = std::max(h, t->getHeight());
	}
	return h;
}

inline int Sprite::getHeight (Layer layer) const
{
	if (_currentFrame == -1)
		return 0;
	const TexturePtr t = getActiveTexture(layer);
	if (!t) {
		return 0;
	}
	return t->getHeight();
}

inline bool Sprite::isLoop () const
{
	return _loop;
}

inline void Sprite::setLoop (bool loop)
{
	_loop = loop;
}

inline void Sprite::setFPS (float fps)
{
	_fps = fps;
}

inline void Sprite::setActive (int frame, bool active)
{
	assert(frame <= _active.size());
	assert(frame > 0);
	_active[frame - 1] = active;
}

inline bool Sprite::isActive (int frame) const
{
	assert(frame <= _active.size());
	assert(frame > 0);
	return _active[frame - 1];
}

inline void Sprite::setDelay (int frame, int delay)
{
	assert(frame <= _delays.size());
	assert(frame > 0);
	_delays[frame - 1] = delay;
}

inline int Sprite::getFrameCount () const
{
	return _frameCount;
}

typedef SharedPtr<Sprite> SpritePtr;
