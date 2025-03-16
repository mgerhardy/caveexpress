#include "Rain.h"
#include "common/Math.h"

Rain::Rain(IParticleEnvironment &env) : Particle(env) {
	_texture = loadTexture("snow-01");
	const float s = randBetweenf(0.1f, 0.6f);
	_scale = vec2(s * 4.f, s * 0.2f);
	_alpha = randBetweenf(0.4f, 0.7f);
	_wind = env.getWind() * 0.2f;
	_angle = -90.f - 30.f * _env.getWind() + randBetweenf(-5.f, 5.f);
	_omega = 0.0f;
	random();
}

void Rain::random() {
	float a = DegreesToRadians(_angle), d = randBetweenf(0.4f, 0.8f);
	_v = vec2(-cosf(a) * d, -sinf(a) * d);
}
