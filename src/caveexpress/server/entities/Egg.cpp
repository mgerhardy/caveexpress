#include "Egg.h"
#include "caveexpress/shared/constants/Density.h"
#include "caveexpress/shared/CaveExpressSoundType.h"
#include "caveexpress/server/map/Map.h"

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
	b2PolygonShape sd;
	sd.SetAsBox(_size.x / 2.0f, _size.y / 2.0f);

	b2FixtureDef fd;
	fd.shape = &sd;
	fd.density = DENSITY_EGG;
	fd.friction = 0.0f;
	fd.restitution = 0.0f;

	b2BodyDef bd;
	bd.position.Set(_x, _y);
	bd.type = b2_dynamicBody;
	bd.fixedRotation = false;

	_map.addToWorld(fd, bd, this);
	_map.addEntity(this);
}
