#include "Dirt.h"

Dirt::Dirt(IParticleEnvironment &env) : Particle(env) {
	_texture = loadTexture("snow-01");
	const float s = randBetweenf(0.3f, 0.5f);
	_scale = vec2(s, s);
	_alpha = randBetweenf(0.5f, 0.7f);
	_wind = env.getWind() * 0.3f;
	random();
}

void Dirt::random() {
	_v = vec2(randBetweenf(-0.03f, 0.03f) + _wind, randBetweenf(0.5f, 0.16f));
	_omega = randBetweenf(-0.3f, 0.3f);
}
