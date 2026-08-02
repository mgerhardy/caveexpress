#include "Egg.h"
#include "caveexpress/shared/constants/Density.h"
#include "caveexpress/shared/CaveExpressSoundType.h"
#include "caveexpress/server/map/Map.h"

namespace caveexpress {

Egg::Egg (Map& map, gridCoord x, gridCoord y) :
		CollectableEntity(EntityTypes::EGG, map), _x(x), _y(y)
{
}

Egg::~Egg ()
{
}

void Egg::onSpawn ()
{
	CollectableEntity::onSpawn();
	_map.sendSound(0, SoundTypes::SOUND_FRUIT_SPAWN, getPos());
}

void Egg::createBody ()
{
	PhysicsFixtureDef fd;
	fd.shapeType = PhysicsShapeType::Polygon;
	fd.useBox = true;
	fd.boxHalfWidth = _size.x / 2.0f;
	fd.boxHalfHeight = _size.y / 2.0f;
	fd.density = DENSITY_EGG;
	fd.friction = 0.0f;
	fd.restitution = 0.0f;

	PhysicsBodyDef bd;
	bd.position.set(_x, _y);
	bd.type = PhysicsBodyType::Dynamic;
	bd.fixedRotation = false;

	_map.addToWorld(fd, bd, this);
	_map.addEntity(this);
}

}
