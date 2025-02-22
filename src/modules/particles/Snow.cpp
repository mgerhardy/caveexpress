#include "Snow.h"
#include "common/Log.h"

Snow::Snow(IParticleEnvironment &env) : Particle(env), _waterSurface(0) {
	_texture = loadTexture("snow-01");
	float s = randBetweenf(0.1f, 0.6f);
	_scale = vec2(s, s);
	random();
}

void Snow::random() {
	_v = vec2(randBetweenf(-0.03f, 0.09f), randBetweenf(0.06f, 0.12f));
	_omega = randBetweenf(-0.3f, 0.6f);
}

void Snow::init() {
	_pos.x = (float)(rand() % _env.getPixelWidth());
	_pos.y = (float)(rand() % _env.getPixelHeight());
	_waterSurface = _env.getWaterSurface();
	Log::error(LogCategory::LOG_PARTICLES, "Snow::init: %i:%i at water height: %i", (int)_pos.x, (int)_pos.y, _waterSurface);
}

void Snow::run() {
	// the water height might change, so update this
	_waterSurface = _env.getWaterSurface();

	// snow has reached the water surface
	if (_pos.y >= (float)(_waterSurface - _texture->getHeight())) {
		_pos.x = (float)(rand() % _env.getPixelWidth());
		_pos.y = (float)(rand() % (_env.getPixelHeight() / 22));
		random();
	}
}
