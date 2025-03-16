#pragma once

#include "Particle.h"

class Leaf : public Particle {

public:
	explicit Leaf (IParticleEnvironment& env);

	void run () override;
	void random () override;
};
