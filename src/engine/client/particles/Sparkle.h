#pragma once

#include "engine/client/particles/Particle.h"

class Sparkle : public Particle {
private:
	int _waterSurface;
	int _startX;
	int _startY;
	int _sizeW;
	int _sizeH;
	int _half;
	int _height;
public:
	Sparkle (IParticleEnvironment& env, int startX, int startY, int sizeW, int sizeH);
	void run () override;
	void init () override;
};
