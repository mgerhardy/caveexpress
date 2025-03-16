#include "Particle.h"
#include "common/Log.h"

Particle::Particle(IParticleEnvironment& env) :
		_env(env), _active(true), _time(0), _deltaTime(0.0f), _alpha(1.0f), _fps(1.0f),
		_tps(1.0f), _lastFrame(0.0f), _lastThink(0.0f), _t(0.0f), _life(0.0f), _omega(0.0f), _angle(0), _scale(1.0f, 1.0f),
		_wind(0.f), _waterSurface(1.0f)
{
}

Particle::~Particle ()
{
}

void Particle::init ()
{
	_waterSurface = _env.getWaterSurface();
	const float w = (float)_env.getPixelWidth();
	const float h = (float)_env.getPixelHeight();
	_pos.x = randBetweenf(0.f, w);
	_pos.y = randBetweenf(0.f, std::min((float)_waterSurface, h));
}

void Particle::newPos()
{
	const float w = (float)_env.getPixelWidth();
	const float h = (float)_env.getPixelHeight();
	const float wind = _env.getWind();
	const float windW = std::max(-1.f, std::min(1.f, wind * 0.5f));
	const float windH = std::min(1.f, fabs(wind) * 0.5f);
	_pos.x = randBetweenf(0.f, w) - windW * 1000.f;
	_pos.y = randBetweenf(0.f, windH * h);
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

void Particle::render (IFrontend* frontend, int x, int y, float zoom) const
{
	if (!_texture || !_texture->isValid())
		return;
	const float fx = (float)x + _pos.x * zoom;
	const float fy = (float)y + _pos.y * zoom;
	const float fw = (float)_texture->getWidth() * zoom * _scale.x;
	const float fh = (float)_texture->getHeight() * zoom * _scale.y;
	// frontend->setColor(_color);
	frontend->renderImage(_texture.get(), (int)fx, (int)fy, (int)fw, (int)fh, (int16_t)_angle, _alpha);
}

void Particle::run ()
{
	// the water height might change, so update this
	_waterSurface = _env.getWaterSurface();

	//  has reached the water surface
	if (_pos.y >= (float)(_waterSurface - _texture->getHeight())) {
		newPos();
		random();
	}
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
	_angle += _omega * _deltaTime;

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
