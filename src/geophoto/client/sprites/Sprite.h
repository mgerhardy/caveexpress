#pragma once

#include "client/textures/Texture.h"
#include "common/Pointers.h"
#include "shared/IFrontend.h"
#include "shared/SpriteDefinition.h"
#include <vector>
#include <string>
#include <stdint.h>

class Sprite {
private:
	typedef std::vector<TexturePtr> AnimationFrames;
	typedef AnimationFrames::iterator AnimationFramesIterator;

	Sprite (const std::string& name, const std::vector<int>& delays,
			const AnimationFrames& textures, int frameCount, float fps, int32_t frameTimeRemaining, bool loop,
			int spriteWidth, int spriteHeight);

	std::vector<int> _delays;

	std::string _name;
	AnimationFrames _textures;
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

	TexturePtr getActiveTexture () const;
	void setFPS (float fps);
	void setTile (bool tile);

	void setDelay (int frame, int delay);
	bool isLoop () const;
	void setLoop (bool loop);
	bool addFrame (const std::string& name, int delay = 0);
	void update (uint32_t deltaTime);

	int getFrameCount () const;
	int getWidth () const;
	int getHeight () const;
	int getMaxWidth () const;
	int getMaxHeight () const;

	// render the sprite fullscreen but at a given y level
	// param[in] y normalized y screen coordinates (0 - EventHandler::HEIGHT)
	bool render (IFrontend *frontend, int y) const;
	// param[in] x normalized x screen coordinates (0 - EventHandler::WIDTH)
	// param[in] y normalized y screen coordinates (0 - EventHandler::HEIGHT)
	bool render (IFrontend *frontend, int x, int y, int w, int h, int16_t angle = 0, float alpha = 1.0f) const;
	// param[in] x normalized x screen coordinates (0 - EventHandler::WIDTH)
	// param[in] y normalized y screen coordinates (0 - EventHandler::HEIGHT)
	bool render (IFrontend *frontend, int x, int y, int16_t angle = 0, float alpha = 1.0f) const;

	void setCurrentFrame (int frame);

	const std::string& getName () const;

	Sprite* copy () const;
};

inline const std::string& Sprite::getName () const
{
	return _name;
}

inline TexturePtr Sprite::getActiveTexture () const
{
	const AnimationFrames& frames = _textures;
	const size_t size = frames.size();
	if (size == 0)
		return TexturePtr();
	if (size < _currentFrame)
		return _textures.at(size - 1);
	return _textures.at(_currentFrame);
}

inline int Sprite::getWidth () const
{
	if (_currentFrame == -1)
		return 0;
	const TexturePtr t = getActiveTexture();
	if (!t) {
		return 0;
	}
	return t->getWidth();
}

inline int Sprite::getMaxWidth () const
{
	const TexturePtr t = getActiveTexture();
	if (!t)
		return 0;
	const int w = std::max(w, t->getWidth());
	return w;
}

inline int Sprite::getMaxHeight () const
{
	const TexturePtr t = getActiveTexture();
	if (!t)
		return 0;
	const int h = std::max(h, t->getHeight());
	return h;
}

inline int Sprite::getHeight () const
{
	if (_currentFrame == -1)
		return 0;
	const TexturePtr t = getActiveTexture();
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
