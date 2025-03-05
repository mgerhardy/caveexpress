#include "Rain.h"

Rain::Rain(IParticleEnvironment &env) : Particle(env), _waterSurface(0) {
	_texture = loadTexture("snow-01");
	float s = randBetweenf(0.1f, 0.6f);
	_scale = vec2(s * 0.2f, s * 4.f);
	_alpha = randBetweenf(0.3f, 0.5f);
	random();
}
void Rain::random() {
	_v = vec2(randBetweenf(-0.01f, 0.01f), randBetweenf(0.3f, 0.6f));
}

void Rain::init() {
	_waterSurface = _env.getWaterSurface();
	_pos.x = rand() % _env.getPixelWidth();
	_pos.y = rand() % std::min(_waterSurface, _env.getPixelHeight());
}

void Rain::run() {
	// the water height might change, so update this
	_waterSurface = _env.getWaterSurface();

	// Rain has reached the water surface
	if (_pos.y >= _waterSurface - _texture->getHeight()) {
		_pos.x = rand() % _env.getPixelWidth();
		_pos.y = rand() % (_env.getPixelHeight() / 32); // top
		random();
	}
}
