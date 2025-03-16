#pragma once

#include "Particle.h"

class Snow : public Particle {

public:
	explicit Snow(IParticleEnvironment &env);

	void random () override;
};
