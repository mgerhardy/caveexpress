#pragma once

#include "Particle.h"
#include <vector>

class IFrontend;

class ParticleSystem {
private:
	typedef std::vector<ParticlePtr> Particles;
	typedef Particles::iterator ParticlesIter;
	typedef Particles::const_iterator ParticlesConstIter;
	Particles _particles;
	int _maxParticles;
public:
	explicit ParticleSystem(int maxParticles);
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
