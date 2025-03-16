#include "Leaf.h"
#include "common/Log.h"
#include "common/String.h"

#define LEAFTYPES 1

Leaf::Leaf(IParticleEnvironment &env)
	: Particle(env) {
	const int i = rand() % LEAFTYPES;
	_texture = loadTexture(string::format("leaf-%02i", i + 1));
	_v = vec2(0.0f, -0.04f);
	_omega = 0.2f;
}

void Leaf::run() {
	const float magnitude = 0.1f;
	const float amplitude = 0.5f;
	_v.x = magnitude * sinf(_v.y * amplitude);
}
