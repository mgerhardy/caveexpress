#include "Snow.h"

Snow::Snow(IParticleEnvironment &env) : Particle(env) {
	_texture = loadTexture("snow-01");
	const float s = randBetweenf(0.1f, 0.6f);
	_scale = vec2(s, s);
	_wind = env.getWind() * 0.2f;
	random();
}

void Snow::random() {
	_v = vec2(randBetweenf(-0.06f, 0.06f) + _wind, randBetweenf(0.06f, 0.12f));
	_omega = randBetweenf(-0.3f, 0.6f);
}
