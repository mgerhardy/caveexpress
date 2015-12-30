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
	float offset;
	const float vx = entity->getLinearVelocity().x;
	if (vx < 0.0f)
		offset = -entity->getSize().x / 3.0f;
	else if (vx > 0.0f)
		offset = entity->getSize().x / 3.0f;
	else
		offset = 0.0f;
	return b2Vec2(entity->getPos().x + offset, _map.getWaterHeight());
}

void WorldParticle::checkParticleGeneratingContacts ()
{
	const float threshold = 0.45;
	for (ContactsIter it = _contacts.begin(); it != _contacts.end(); ++it) {
		const IEntity* entity = *it;
		b2Body* body = entity->getBodies()[0];
		const b2Vec2& v = body->GetLinearVelocity();
		const float speed = v.Length();
		if (speed > threshold)
			spawnParticle(getSpawnPosition(entity), v);
	}
}

WorldParticle::SimpleParticle* WorldParticle::createParticleBody ()
{
	SimpleParticle* p = new SimpleParticle();
	p->life = _lifetime;

	b2BodyDef bd;
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
	p->body->SetUserData(this);
	p->body->CreateFixture(&fd);
	_bodies.push_back(p->body);

	return p;
}

void WorldParticle::onPreSolve (b2Contact* contact, IEntity* entity, const b2Manifold* oldManifold)
{
	if (_particleType != WATER)
		return;

	const b2Fixture* fixture = contact->GetFixtureA();
	const bool useBodyA = fixture->GetBody()->GetUserData() == this || fixture->GetUserData() == this;
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
	b->SetActive(true);
	b2Vec2 vel = v;
	vel *= 0.1;
	vel.x += randBetweenf(-2, 2);
	vel.y += randBetweenf(-2, 2);
	b->SetTransform(pos, 0);
	b->SetLinearVelocity(vel);
	b->SetGravityScale(1.0f);
	b->SetLinearDamping(4);
	b->SetAngularDamping(4);
}

void WorldParticle::update (uint32_t deltaTime)
{
	IEntity::update(deltaTime);
	_particlesDirty = _time - _lastDirtyTime > 100;
	if (_particlesDirty) {
		_lastDirtyTime = _time;
	}

	for (ParticlesIter i = _particles.begin(); i != _particles.end(); ++i) {
		SimpleParticle* p = *i;
		if (p->life < deltaTime) {
			p->life = 0;
			p->body->SetActive(false);
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
