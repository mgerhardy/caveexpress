#pragma once

#include "Particle.h"

class Sand : public Particle {

public:
	explicit Sand(IParticleEnvironment &env);

	void random () override;
};
