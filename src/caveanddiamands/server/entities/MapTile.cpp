#include "MapTile.h"
#include "caveanddiamonds/server/map/Map.h"
#include "engine/common/SpriteDefinition.h"

MapTile::MapTile (Map& map, int col, int row, const EntityType &type) :
		IEntity(type, map, col, row)
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
