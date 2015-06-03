#include "ParticleSystem.h"
#include "common/IFrontend.h"

ParticleSystem::ParticleSystem (int maxParticles) :
		_maxParticles(maxParticles)
{
}

ParticleSystem::~ParticleSystem ()
{
	clear();
}

void ParticleSystem::render (IFrontend* frontend, int x, int y, float zoom) const
{
	for (ParticlesConstIter i = _particles.begin(); i != _particles.end(); ++i) {
		const ParticlePtr& t = *i;
		t->render(frontend, x, y, zoom);
	}
}

void ParticleSystem::clear ()
{
	_particles.clear();
}

void ParticleSystem::update (uint32_t deltaTime)
{
	for (ParticlesIter i = _particles.begin(); i != _particles.end();) {
		ParticlePtr& t = *i;
		if (!t->update(deltaTime))
			i = _particles.erase(i);
		else
			++i;
	}
}

bool ParticleSystem::spawn (const ParticlePtr& particle)
{
	if (_particles.size() >= _maxParticles)
		return false;
	_particles.push_back(particle);
	particle->init();
	return true;
}

void ParticleSystem::remove (const ParticlePtr& particle)
{
	ParticlesIter i = std::find_if(_particles.begin(), _particles.end(), SharedPtrIf<Particle>(particle));
	if (i == _particles.end())
		return;
	_particles.erase(i);
}
