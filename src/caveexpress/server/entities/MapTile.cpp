#include "MapTile.h"
#include "caveexpress/server/map/Map.h"
#include "caveexpress/shared/constants/Density.h"
#include "common/SpriteDefinition.h"
#include <SDL_assert.h>

namespace caveexpress {

MapTile::MapTile (Map& map, const std::string& spriteID, gridCoord gridX, gridCoord gridY, const EntityType &type) :
		IEntity(type, map), _gridX(gridX), _gridY(gridY), _gridWidth(1.0f), _gridHeight(1.0f), _pos(b2Vec2_zero)
{
	SDL_assert_always(EntityTypes::isMapTile(_type));
	setSpriteID(spriteID);

	setGridDimensions(1, 1, 0);
}

MapTile::~MapTile ()
{
}

bool MapTile::isUnderWater () const
{
	return _map.getWaterHeight() < getPos().y;
}

bool MapTile::shouldCollide (const IEntity* entity) const
{
	return entity->isSolid();
}

void MapTile::createBody ()
{
	b2PolygonShape sd;
	sd.SetAsBox(_size.x / 2.0f, _size.y / 2.0f);

	b2FixtureDef fd;
	fd.userData = nullptr;
	fd.shape = &sd;
	fd.density = DENSITY_STONE;
	fd.friction = 0.2f;
	fd.restitution = 0.0f;

	b2BodyDef bd;
	bd.userData = nullptr;
	bd.position.Set(_pos.x, _pos.y);
	bd.type = b2_staticBody;
	bd.fixedRotation = true;
	bd.angle = DegreesToRadians(_angle);

	_map.addToWorld(fd, bd, this);
	// override the size again.
	setGridDimensions(_gridWidth, _gridHeight, _angle);
}

// returns the center of the object
// this is not the screen coordinate!
const b2Vec2& MapTile::getPos () const
{
	return _pos;
}

SpriteDefPtr MapTile::getSpriteDef () const
{
	const SpriteDefPtr& def = SpriteDefinition::get().getSpriteDefinition(getSpriteID());
	return def;
}

}
