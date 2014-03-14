#pragma once

#include "engine/client/particles/Particle.h"

class ClientMap;
class IFrontend;

class Snow: public Particle {
private:
	int _waterSurface;
public:
	Snow(IParticleEnvironment& env);
	void run () override;
	void init () override;
};
