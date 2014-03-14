#pragma once

#include "engine/client/particles/Particle.h"

class Bubble : public Particle {
private:
	int _waterSurface;
	int _waterGround;
	int _waterHeight;
	int _waterWidth;
public:
	Bubble (IParticleEnvironment& env);
	void run () override;
	void init () override;
};
