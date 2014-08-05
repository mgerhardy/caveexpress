#include "Sprite.h"
#include "engine/common/IFrontend.h"
#include "engine/common/Logger.h"
#include "engine/client/ui/UI.h"

Sprite::Sprite (const std::string& name) :
		_name(name), _currentFrame(-1), _frameCount(0), _fps(20.0f), _frameTimeRemaining(1000.0f / _fps), _loop(
				true), _spriteWidth(0), _spriteHeight(0)
{
}

Sprite::Sprite (const std::string& name, const std::vector<int>& delays, const std::vector<bool>& active,
		const AnimationFrames* textures, int frameCount, float fps, int32_t frameTimeRemaining, bool loop,
		int spriteWidth, int spriteHeight) :
		_name(name), _delays(delays), _active(active), _currentFrame(-1), _frameCount(frameCount), _fps(
				fps), _frameTimeRemaining(frameTimeRemaining), _loop(loop), _spriteWidth(spriteWidth), _spriteHeight(
				spriteHeight)
{
	for (int i = 0; i < MAX_LAYERS; ++i)
		_textures[i] = textures[i];

	_currentFrame = 0;
}

Sprite::~Sprite ()
{
	for (int i = 0; i < MAX_LAYERS; ++i)
		_textures[i].clear();
}

Sprite* Sprite::copy () const
{
	debug(LOG_CLIENT, "copy sprite " + _name);
	return new Sprite(_name, _delays, _active, _textures, _frameCount, _fps, _frameTimeRemaining, _loop, _spriteWidth, _spriteHeight);
}

void Sprite::setCurrentFrame (int frame)
{
	if (frame < 0 || frame >= _frameCount) {
		error(LOG_CLIENT, String::format("frame number invalid for %s (%i/%i)", _name.c_str(), frame, _frameCount));
		return;
	}

	_currentFrame = frame;
}

bool Sprite::addFrame (Layer layer, const std::string& name, int delay, bool active)
{
	if (name.empty())
		return false;
	const TexturePtr& t = UI::get().loadTexture(name);
	if (!t->isValid()) {
		return false;
	}
	_textures[layer].push_back(t);
	_delays.push_back(delay);
	_active.push_back(active);
	_frameCount = _textures[layer].size();
	_spriteWidth = std::max(t->getWidth(), _spriteWidth);
	_spriteHeight = std::max(t->getHeight(), _spriteHeight);
	_currentFrame = 0;
	return true;
}

void Sprite::update (uint32_t deltaTime)
{
	if (_frameCount <= 1)
		return;
	if (_fps <= 0.00001f)
		return;

	_frameTimeRemaining -= deltaTime;
	if (_frameTimeRemaining < 0) {
		if (_loop || _currentFrame + 1 < _frameCount) {
			_currentFrame++;
			_currentFrame %= _frameCount;
		}
		const int newFrameTime = _frameTimeRemaining + 1000.0f / _fps + _delays[_currentFrame];
		_frameTimeRemaining = newFrameTime;
	}
}

bool Sprite::render (IFrontend *frontend, Layer layer, int x, int y, int w, int h, int16_t angle, float alpha) const
{
	if (_currentFrame == -1)
		return false;
	// culling
	if (x + w < 0 || y + h < 0)
		return false;

	const TexturePtr t = getActiveTexture(layer);
	if (!t)
		return false;
	frontend->renderImage(t.get(), x, y, w, h, angle, alpha);
	return true;
}

bool Sprite::render (IFrontend *frontend, Layer layer, int y) const
{
	if (_currentFrame == -1)
		return false;
	// culling the sprites
	if (y >= frontend->getHeight())
		return false;

	const TexturePtr t = getActiveTexture(layer);
	if (!t)
		return false;
	if (y + t->getHeight() < 0)
		return false;

	frontend->renderImage(t.get(), 0, y, frontend->getWidth(), frontend->getHeight() - y, 0, 1.0f);
	return true;
}

bool Sprite::render (IFrontend *frontend, Layer layer, int x, int y, int16_t angle, float alpha) const
{
	return render(frontend, layer, x, y, 1.0f, angle, alpha);
}

bool Sprite::render (IFrontend *frontend, Layer layer, int x, int y, float zoom, int16_t angle, float alpha) const
{
	if (_currentFrame == -1)
		return false;

	// culling the sprites
	if (x >= frontend->getWidth())
		return false;

	const int h = _spriteHeight;
	if (y >= frontend->getHeight())
		return false;

	const int w = _spriteWidth;
	if (x + w < 0)
		return false;
	else if (y + h < 0)
		return false;

	const TexturePtr t = getActiveTexture(layer);
	if (!t)
		return false;

	// the positions are the center of the sprites - so we have to offset them here
	frontend->renderImage(t.get(), x, y, t->getWidth() * zoom, t->getHeight() * zoom, angle, alpha);
	return true;
}
