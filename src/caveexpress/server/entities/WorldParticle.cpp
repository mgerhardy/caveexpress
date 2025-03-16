#include "WorldParticle.h"
#include "caveexpress/server/map/Map.h"
#include <SDL_assert.h>

namespace caveexpress {

WorldParticle::WorldParticle(Map& map, WorldParticleType type, int maxParticles, float density, const b2Vec2& size, uint32_t lifetime) :
		IEntity(EntityTypes::PARTICLE, map), _particleType(type), _maxParticles(
				maxParticles), _nextParticleIndex(0), _density(density), _lifetime(lifetime), _lastDirtyTime(0), _particlesDirty(false)
{
	_size = size;
}

WorldParticle::~WorldParticle()
{
	for (ParticlesIter i = _particles.begin(); i != _particles.end(); ++i) {
		delete *i;
	}
}

bool WorldParticle::shouldCollide (const IEntity *entity) const
{
	switch (_particleType) {
	case WATER:
		return entity->isWater() || entity->isParticle();
	default:
		return false;
	}
}

b2Vec2 WorldParticle::getSpawnPosition (const IEntity* entity) const
{
	return b2Vec2(entity->getPos().x, entity->getPos().y);
}

void WorldParticle::checkParticleGeneratingContacts ()
{
	const float threshold = 0.35f;
	for (ContactsIter it = _contacts.begin(); it != _contacts.end(); ++it) {
		const IEntity* entity = *it;
		b2Body* body = entity->getBodies()[0];
		const b2Vec2& v = body->GetLinearVelocity();
		const float speed = v.Length();
		if (speed > threshold) {
			spawnParticle(getSpawnPosition(entity), v);
		}
	}
}

WorldParticle::SimpleParticle* WorldParticle::createParticleBody ()
{
	SimpleParticle* p = new SimpleParticle();
	p->life = _lifetime;

	b2BodyDef bd;
	bd.userData.pointer = (uintptr_t)this;
	bd.type = b2_dynamicBody;
	bd.fixedRotation = false;

	b2PolygonShape shape;
	shape.SetAsBox(_size.x / 2.0f, _size.y / 2.0f);

	b2FixtureDef fd;
	fd.density = _density;
	fd.shape = &shape;
	fd.friction = 2.0f;
	fd.restitution = 0.1f;

	p->body = _map.getWorld()->CreateBody(&bd);
	p->body->CreateFixture(&fd);
	_bodies.push_back(p->body);

	return p;
}

void WorldParticle::onPreSolve (b2Contact* contact, IEntity* entity, const b2Manifold* oldManifold)
{
	if (_particleType != WATER)
		return;

	b2Fixture* fixture = contact->GetFixtureA();
	const bool useBodyA = fixture->GetBody()->GetUserData().pointer == (uintptr_t)this || fixture->GetUserData().pointer == (uintptr_t)this;
	const b2Body *body = useBodyA ? fixture->GetBody() : contact->GetFixtureB()->GetBody();
	const SimpleParticle* p = _particleReverseMap[body];
	SDL_assert(p);
	const bool enabled = p->life > 0;
	contact->SetEnabled(enabled);
}

void WorldParticle::spawnParticle (const b2Vec2& pos, const b2Vec2& v)
{
	int currentParticleIndex = _nextParticleIndex;
	SimpleParticle* p;
	if (static_cast<int>(_particles.size()) < _maxParticles) {
		// add a new one
		_particles.push_back(createParticleBody());
		_nextParticleIndex++;
		p = _particles[currentParticleIndex];
		_particleReverseMap[p->body] = p;
	} else {
		// reuse oldest
		currentParticleIndex %= _maxParticles;
		_nextParticleIndex = (_nextParticleIndex + 1) % _maxParticles;
		p = _particles[currentParticleIndex];
	}

	p->life = _lifetime;

	b2Body* b = p->body;
	b->SetEnabled(true);
	b2Vec2 vel = v;
	if (_particleType == WATER) {
		vel *= 1.7f;
		vel.y *= -1.f;  // splash
		vel.x = std::min(2.f, std::max(-2.f, vel.x));
		vel.y = std::min(2.f, std::max(-2.f, vel.y));
		vel.x += randBetweenf(-1.f, 1.f);
		vel.y += randBetweenf(-1.f, 1.f);
	}
	b->SetLinearVelocity(vel);

	b2Vec2 pos2 = pos;
	pos2.x += randBetweenf(-0.2f, 0.2f);
	pos2.y += randBetweenf(-0.2f, 0.2f);
	b->SetTransform(pos2, 0);

	b->SetGravityScale(1.0f);
	b->SetLinearDamping(1.f);
	b->SetAngularDamping(1.f);
}

void WorldParticle::update (uint32_t deltaTime)
{
	IEntity::update(deltaTime);
	_particlesDirty = _time - _lastDirtyTime > 10;
	if (_particlesDirty) {
		_lastDirtyTime = _time;
	}

	for (ParticlesIter i = _particles.begin(); i != _particles.end(); ++i) {
		SimpleParticle* p = *i;
		if (p->life < deltaTime) {
			p->life = 0;
			p->body->SetEnabled(false);
			continue;
		}
		p->life -= deltaTime;
	}

	checkParticleGeneratingContacts();
}

bool WorldParticle::isDirty () const
{
	const bool dirty = _particlesDirty;
	_particlesDirty = false;
	return dirty;
}

}
