#pragma once

#include "Particle.h"

class Dirt : public Particle {

public:
	explicit Dirt(IParticleEnvironment &env);

	void random () override;
};
