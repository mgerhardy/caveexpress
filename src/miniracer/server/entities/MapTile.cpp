#include "MapTile.h"
#include "miniracer/server/map/Map.h"
#include "common/SpriteDefinition.h"

namespace miniracer {

MapTile::MapTile (Map& map, int x, int y, const EntityType &type) :
		IEntity(type, map)
{
	setSpriteID(_type.name);
}

MapTile::~MapTile ()
{
}

SpriteDefPtr MapTile::getSpriteDef () const
{
	const SpriteDefPtr& def = SpriteDefinition::get().getSpriteDefinition(getSpriteID());
	return def;
}

}
