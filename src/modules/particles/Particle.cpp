#include "Particle.h"
#include "common/Log.h"

Particle::Particle(IParticleEnvironment& env) :
		_env(env), _active(true), _time(0), _deltaTime(0.0f), _alpha(1.0f), _angle(0), _fps(1.0f),
		_tps(1.0f), _lastFrame(0.0f), _lastThink(0.0f), _t(0.0f), _life(0.0f), _omega(0.0f), _scale(1.0f, 1.0f)
{
}

Particle::~Particle ()
{
}

TexturePtr Particle::loadTexture (const std::string& image) const
{
	const TexturePtr &t = _env.loadTexture(image);
	if (!t || !t->isValid()) {
		Log::error(LOG_PARTICLES, "loadTexture: failed to load texture %s", image.c_str());
	}
	return t;
}

inline void Particle::advanceVector (const vec2& veca, const float scale, const vec2& vecb, vec2& outVector) const
{
	outVector.x = veca.x + scale * vecb.x;
	outVector.y = veca.y + scale * vecb.y;
}

bool Particle::update (uint32_t deltaTime)
{
	run();

	if (!_active)
		return true;

	_deltaTime = deltaTime;
	_time += _deltaTime;
	_t = _time * 0.001f;
	_lastThink += _deltaTime;
	_lastFrame += _deltaTime;
	advanceVector(_pos, 0.5f * _deltaTime * _deltaTime, _a, _pos);
	advanceVector(_pos, _deltaTime, _v, _pos);
	advanceVector(_v, _deltaTime, _a, _v);
	_angle = _omega + _deltaTime * (float)_angle;

	while (_tps > 0.0f && _lastThink * _tps >= 1.0f) {
		think();
		_lastThink -= 1.0f / _tps;
	}

	while (_fps > 0.0f && _lastFrame * _fps >= 1.0f) {
		// TODO: sprite
		_lastFrame -= 1.0f / _fps;
	}

	if (_life > 0.0f && _t >= _life)
		return false;

	return true;
}

void Particle::render (IFrontend* frontend, int x, int y, float zoom) const
{
	if (!_texture || !_texture->isValid())
		return;
	const float fx = (float)x + _pos.x * zoom;
	const float fy = (float)y + _pos.y * zoom;
	const float fw = (float)_texture->getWidth() * zoom * _scale.x;
	const float fh = (float)_texture->getHeight() * zoom * _scale.y;
	if (fx + fw < 0.0f || fy + fh < 0.0f || fx > (float)frontend->getWidth() || fy > (float)frontend->getHeight()) {
		return;
	}
	frontend->renderImage(_texture.get(), (int)fx, (int)fy, (int)fw, (int)fh, _angle, _alpha);
}
