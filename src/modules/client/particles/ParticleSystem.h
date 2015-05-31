#pragma once

#include "common/IFrontend.h"
#include "Particle.h"
#include <vector>

class ParticleSystem {
private:
	typedef std::vector<ParticlePtr> Particles;
	typedef Particles::iterator ParticlesIter;
	typedef Particles::const_iterator ParticlesConstIter;
	Particles _particles;
	int _maxParticles;
public:
	ParticleSystem(int maxParticles);
	virtual ~ParticleSystem();

	void clear ();

	void render (IFrontend* frontend, int x, int y, float zoom) const;
	void update (uint32_t deltaTime);
	bool spawn (const ParticlePtr& particle);
	void remove (const ParticlePtr& particle);
	bool hasParticles () const ;
	int getParticleAmount () const;
};

inline int ParticleSystem::getParticleAmount () const
{
	return _particles.size();
}

inline bool ParticleSystem::hasParticles () const
{
	return !_particles.empty();
}
