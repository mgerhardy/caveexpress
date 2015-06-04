#pragma once

#include "Particle.h"

class Snow: public Particle {
private:
	int _waterSurface;
public:
	explicit Snow(IParticleEnvironment& env);
	void run () override;
	void init () override;
};
