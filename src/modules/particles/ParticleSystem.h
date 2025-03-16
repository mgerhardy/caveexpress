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
	const int _maxParticles;
	float _wind = 0.0f;
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

	void setWind(float wind);
};

inline void ParticleSystem::setWind(float wind)
{
	_wind = wind;
}

inline int ParticleSystem::getParticleAmount () const
{
	return static_cast<int>(_particles.size());
}

inline bool ParticleSystem::hasParticles () const
{
	return !_particles.empty();
}
