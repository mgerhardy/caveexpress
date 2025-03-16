#include "Sand.h"

Sand::Sand(IParticleEnvironment &env) : Particle(env) {
	_texture = loadTexture("snow-01");
	const float s = randBetweenf(0.2f, 0.4f);
	_scale = vec2(s, s);
	_alpha = randBetweenf(0.7f, 0.9f);
	_wind = env.getWind() * 0.8f;
	random();
}

void Sand::random() {
	_v = vec2(randBetweenf(-0.01f, 0.01f) + _wind, randBetweenf(0.05f, 0.2f));
}
