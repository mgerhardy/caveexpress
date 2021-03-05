#include "Fruit.h"
#include "caveexpress/shared/constants/Density.h"
#include "caveexpress/server/map/Map.h"
#include "caveexpress/shared/CaveExpressSoundType.h"
#include <SDL_assert.h>

namespace caveexpress {

Fruit::Fruit (Map& map, const EntityType& type, gridCoord x, gridCoord y) :
		CollectableEntity(type, map), _x(x), _y(y)
{
	SDL_assert(EntityTypes::isFruit(type));
	if (EntityTypes::isApple(type))
		_spriteAlignment = ENTITY_ALIGN_MIDDLE_CENTER;
}

Fruit::~Fruit ()
{
}

void Fruit::onSpawn ()
{
	CollectableEntity::onSpawn();
	_map.sendSound(0, SoundTypes::SOUND_FRUIT_SPAWN, getPos());
}

void Fruit::createBody ()
{
	b2PolygonShape sd;
	sd.SetAsBox(_size.x / 2.0f, _size.y / 2.0f);

	b2FixtureDef fd;
	fd.shape = &sd;
	fd.density = DENSITY_FRUIT;
	fd.friction = 0.0f;
	fd.restitution = 0.0f;

	b2BodyDef bd;
	bd.position.Set(_x, _y);
	bd.type = b2_dynamicBody;
	bd.fixedRotation = false;
	bd.angularDamping = 1.0f;

	_map.addToWorld(fd, bd, this);
	_map.addEntity(this);
}

}
