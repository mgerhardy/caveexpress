#include "Leaf.h"
#include "common/String.h"

#define LEAFTYPES 1

Leaf::Leaf(IParticleEnvironment &env)
	: Particle(env) {
	const int i = rand() % LEAFTYPES;
	_texture = loadTexture(string::format("leaf-%02i", i + 1));

	const float s = randBetweenf(0.5f, 1.0f);
	_scale = vec2(s, s);
	_wind = env.getWind() * 0.2f;
	random();
}

void Leaf::random() {
	_v = vec2(randBetweenf(-0.1f, 0.1f) + _wind, randBetweenf(0.2f, 0.4f));
	_omega = randBetweenf(-1.f, 1.f);
}

void Leaf::run() {
	// wobbly fall animation
	_pos.x += 3.5f * sinf(_pos.y * 0.1384f + _t*0.12f * _omega) * cosf(_pos.x * 0.1542f + _t*0.14f);
	_pos.y += 2.5f * sinf(_pos.x * 0.1357f + _t*0.15f * _omega) * cosf(_pos.y * 0.1575f + _t*0.13f);
	Particle::run();
}
