#include "WorldParticle.h"
#include "caveexpress/server/map/Map.h"
#include "caveexpress/shared/WorldParticleType.h"
#include <SDL_assert.h>

namespace caveexpress {

WorldParticle::WorldParticle(Map& map, WorldParticleType type, int maxParticles, float density, const PhysicsVec2& size, uint32_t lifetime) :
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

bool WorldParticle::shouldApplyWind () const
{
	return _particleType != WATER;
}

bool WorldParticle::shouldCollide (const IEntity *entity) const
{
	switch (_particleType) {
	case WATER:
		return entity->isWater() || entity->isParticle();
	case LEAF:
		return entity->isSolid() || entity->isParticle();
	default:
		return false;
	}
}

PhysicsVec2 WorldParticle::getSpawnPosition (const IEntity* entity) const
{
	return PhysicsVec2(entity->getPos().x, entity->getPos().y);
}

void WorldParticle::checkParticleGeneratingContacts ()
{
	const float threshold = 0.35f;
	for (ContactsIter it = _contacts.begin(); it != _contacts.end(); ++it) {
		const IEntity* entity = *it;
		PhysicsBody body = entity->getBodies()[0];
		const PhysicsVec2& v = body.getLinearVelocity();
		const float speed = v.length();
		if (speed > threshold) {
			spawnParticle(getSpawnPosition(entity), v);
		}
	}
}

WorldParticle::SimpleParticle* WorldParticle::createParticleBody ()
{
	SimpleParticle* p = new SimpleParticle();
	p->life = _lifetime;

	PhysicsBodyDef bd;
	bd.userData = (uintptr_t)this;
	bd.type = PhysicsBodyType::Dynamic;
	bd.fixedRotation = false;

	PhysicsFixtureDef fd;
	fd.shapeType = PhysicsShapeType::Polygon;
	fd.useBox = true;
	fd.boxHalfWidth = _size.x / 2.0f;
	fd.boxHalfHeight = _size.y / 2.0f;
	fd.density = _density;
	fd.friction = 2.0f;
	fd.restitution = 0.1f;

	p->body = _map.getWorld()->createBody(bd);
	p->body.createFixture(fd);
	_bodies.push_back(p->body);

	return p;
}

void WorldParticle::onPreSolve (PhysicsContact contact, IEntity* entity, const PhysicsManifold& oldManifold)
{
	if (_particleType != WATER)
		return;

	PhysicsFixture fixture = contact.getFixtureA();
	const bool useBodyA = fixture.getBody().getUserData() == (uintptr_t)this || fixture.getUserData() == (uintptr_t)this;
	PhysicsBody body = useBodyA ? fixture.getBody() : contact.getFixtureB().getBody();
	const SimpleParticle* p = _particleReverseMap[body];
	SDL_assert(p);
	const bool enabled = p->life > 0;
	contact.setEnabled(enabled);
}

void WorldParticle::spawnParticle (const PhysicsVec2& pos, const PhysicsVec2& v)
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

	PhysicsBody b = p->body;
	b.setEnabled(true);
	PhysicsVec2 vel = v;
	if (_particleType == WATER) {
		vel *= 1.7f;
		vel.y *= -1.f;  // splash
		vel.x = std::min(2.f, std::max(-2.f, vel.x));
		vel.y = std::min(2.f, std::max(-2.f, vel.y));
		vel.x += randBetweenf(-1.f, 1.f);
		vel.y += randBetweenf(-1.f, 1.f);
	}
	b.setLinearVelocity(vel);

	PhysicsVec2 pos2 = pos;
	pos2.x += randBetweenf(-0.2f, 0.2f);
	pos2.y += randBetweenf(-0.2f, 0.2f);
	b.setTransform(pos2, 0);

	b.setGravityScale(1.0f);
	b.setLinearDamping(1.f);
	b.setAngularDamping(1.f);
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
			p->body.setEnabled(false);
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
