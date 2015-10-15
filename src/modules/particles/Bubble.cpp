#include "Bubble.h"
#include "common/String.h"
#include "common/Log.h"

#define BUBBLETYPES 9

Bubble::Bubble (IParticleEnvironment& env) :
	Particle(env), _waterSurface(0), _waterGround(0), _waterHeight(0), _waterWidth(0)
{
	const int i = rand() % BUBBLETYPES;
	_texture = loadTexture(string::format("bubble-%02i", i + 1));
	_v = vec2(0.0f, -0.04f);
	_omega = 0.2f;
}

void Bubble::init ()
{
	_waterSurface = _env.getWaterSurface();
	_waterGround = _env.getWaterGround();
	_waterHeight = _waterGround - _waterSurface;
	_waterWidth = _env.getWaterWidth();
}

void Bubble::run ()
{
	if (_waterHeight <= 0.0)
		return;
	// the water height might change, so update this
	init();
	const float magnitude = 0.1f;
	const float amplitude = 0.5f;
	_v.x = magnitude * sinf(_v.y * amplitude);

	// bubble has reached the water surface
	if (_s.y <= _waterSurface) {
		_s.x = rand() % _waterWidth;
		_s.y = (_waterSurface + _waterHeight / 2) + (rand() % (_waterHeight / 2));
	}
}
