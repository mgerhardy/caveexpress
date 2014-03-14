#pragma once

#include "caveexpress/server/entities/IEntity.h"
#include "caveexpress/shared/WorldParticleType.h"
#include <vector>
#include <set>

class WorldParticle: public IEntity {
protected:
	struct SimpleParticle {
		b2Body* body;
		uint32_t life;
	};

	WorldParticleType _particleType;
	int _maxParticles;

	typedef std::vector<SimpleParticle*> Particles;
	typedef Particles::iterator ParticlesIter;
	Particles _particles;
	int _nextParticleIndex;
	typedef std::map<const b2Body*, SimpleParticle*> ParticleReverseMap;
	ParticleReverseMap _particleReverseMap;

	typedef std::set<const IEntity*> Contacts;
	typedef Contacts::iterator ContactsIter;
	Contacts _contacts;
	float _density;
	uint32_t _lifetime;
	uint32_t _lastDirtyTime;
	mutable bool _particlesDirty;

	void spawnParticle (const b2Vec2& pos, const b2Vec2& v);
	SimpleParticle* createParticleBody ();
	void checkParticleGeneratingContacts ();
	inline b2Vec2 getSpawnPosition (const IEntity* entity) const;
public:
	WorldParticle (Map& map, WorldParticleType type, int maxParticles, float density, const b2Vec2& size, uint32_t lifetime = 2000);
	virtual ~WorldParticle ();

	void addContact (const IEntity* entity);
	void removeContact (const IEntity* entity);

	int getMaxParticles () const;
	uint32_t getLifeTime () const;
	bool isActive (int index) const;
	uint32_t getRemainingLifetime (int index) const;

	// IEntity
	bool shouldCollide (const IEntity *entity) const override;
	void update (uint32_t deltaTime) override;
	bool isDirty () const override;
	void onPreSolve (b2Contact* contact, IEntity* entity, const b2Manifold* oldManifold) override;
};

inline int WorldParticle::getMaxParticles () const
{
	return _maxParticles;
}

inline void WorldParticle::addContact (const IEntity* entity)
{
	if (entity->isParticle())
		return;
	_contacts.insert(entity);
}

inline void WorldParticle::removeContact (const IEntity* entity)
{
	if (entity->isParticle())
		return;
	_contacts.erase(entity);
}

inline bool WorldParticle::isActive (int index) const
{
	return getRemainingLifetime(index) > 0;
}

inline uint32_t WorldParticle::getRemainingLifetime (int index) const
{
	return _particles[index]->life;
}

inline uint32_t WorldParticle::getLifeTime () const
{
	return _lifetime;
}
