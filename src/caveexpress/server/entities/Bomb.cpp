#include "Bomb.h"
#include "caveexpress/server/map/Map.h"
#include "caveexpress/shared/constants/Density.h"

namespace caveexpress {

Bomb::Bomb (Map& map, gridCoord x, gridCoord y, const IEntity *creator) :
		CollectableEntity(EntityTypes::BOMB, map, creator), _x(x), _y(y), _blastPower(1500.0f), _numRays(20), _linearDamping(
				10.0f), _explodeTimer(0), _destroyTimer(0)
{
	setAnimationType(Animations::ANIMATION_IDLE);
}

Bomb::~Bomb ()
{
	_map.getTimeManager().clearTimeout(_explodeTimer);
	_map.getTimeManager().clearTimeout(_destroyTimer);
	for (ParticlesIter i = _particles.begin(); i != _particles.end(); ++i) {
		_map.getWorld()->destroyBody(*i);
	}
	_particles.clear();
}

void Bomb::initiateDetonation ()
{
	// TODO: init animation?
	_explodeTimer = _map.getTimeManager().setTimeout(2000, this, &Bomb::explode);
}

void Bomb::createBody ()
{
	PhysicsFixtureDef fd;
	fd.shapeType = PhysicsShapeType::Polygon;
	fd.useBox = true;
	fd.boxHalfWidth = _size.x / 2.0f;
	fd.boxHalfHeight = _size.y / 2.0f;
	fd.density = DENSITY_BOMB;
	fd.friction = 0.4f;
	fd.restitution = 0.2f;

	PhysicsBodyDef bd;
	bd.position.set(_x, _y);
	bd.type = PhysicsBodyType::Dynamic;
	bd.fixedRotation = false;
	bd.angularDamping = 1.0f;

	_map.addToWorld(fd, bd, this);
	_map.addEntity(this);
}

void Bomb::explode ()
{
	setState(EntityState::ENTITY_DESTROYED);

	const PhysicsVec2& center = getPos();

	PhysicsBodyDef bd;
	bd.type = PhysicsBodyType::Dynamic;
	// rotation not necessary
	bd.fixedRotation = true;
	// prevent tunneling at high speed
	bd.bullet = true;
	// drag due to moving through air
	bd.linearDamping = _linearDamping;
	// ignore gravity
	bd.gravityScale = 0.0f;
	bd.position = center;

	PhysicsFixtureDef fd;
	fd.shapeType = PhysicsShapeType::Circle;
	fd.radius = 0.05f; // very small
	fd.density = DENSITY_BOMB / static_cast<float>(_numRays);
	fd.friction = 0.0f;
	// high restitution to reflect off obstacles
	fd.restitution = 0.99f;
	// particles should not collide with each other
	fd.filter.groupIndex = -1;

	for (int i = 0; i < _numRays; ++i) {
		const float angle = (float)DegreesToRadians(((float)i / (float)_numRays) * 360.0f);
		const PhysicsVec2 rayDir(sinf(angle), cosf(angle));

		bd.linearVelocity = _blastPower * rayDir;
		PhysicsBody body = _map.getWorld()->createBody(bd);
		body.createFixture(fd);
		_particles.push_back(body);
	}

	setAnimationType(Animations::ANIMATION_EXPLODE);
	const int length = 2000; // SpriteDefinition::get().getAnimationLength(_type, getAnimationType());
	_destroyTimer = _map.getTimeManager().setTimeout(length, this, &Bomb::destroy);
}

void Bomb::destroy ()
{
	_remove = true;
}

bool Bomb::shouldApplyWind () const
{
	return true;
}

bool Bomb::shouldCollide (const IEntity *entity) const
{
	if (CollectableEntity::shouldCollide(entity))
		return true;
	return entity->isCollectable();
}

}
