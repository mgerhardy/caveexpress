#include "Sparkle.h"
#include "engine/common/String.h"
#include "engine/common/Logger.h"

#define SPARKLETYPES 3

Sparkle::Sparkle (IParticleEnvironment& env, int startX, int startY, int sizeW, int sizeH) :
	Particle(env), _waterSurface(0), _startX(startX), _startY(startY), _sizeW(sizeW), _sizeH(sizeH)
{
	const int i = rand() % SPARKLETYPES;
	_texture = loadTexture(String::format("sparkle-%02i", i + 1));
	_v = vec2(0.0f, randBetweenf(-0.1f, -0.2f));
	_omega = 0.4f;
	_half = _sizeH / 2.0f;
	_height = _sizeH;
}

void Sparkle::init ()
{
	_waterSurface = _env.getWaterSurface();
}

void Sparkle::run ()
{
	// the water height might change, so update this
	init();
	const float magnitude = 0.1f;
	const float amplitude = 0.5f;
	_v.x = magnitude * sinf(_v.y * amplitude);

	// sparkle is under water
	if (_s.y >= _waterSurface) {
		_active = false;
	} else if (_s.y <= 0.000001f) {
		// init
		_s.y = _startY - rand() % _half;
		_s.x = _startX + rand() % _sizeW;
	} else if (_startY - _s.y > _height) {
		// high enough - respawn
		_s.y = _startY - rand() % _half;
		_s.x = _startX + rand() % _sizeW;
		_v = vec2(0.0f, randBetweenf(-0.1f, -0.2f));
		_height = randBetween(_sizeH, _half);
	}
}
