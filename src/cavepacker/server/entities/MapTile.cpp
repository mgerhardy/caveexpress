#include "MapTile.h"
#include "cavepacker/server/map/Map.h"
#include "engine/common/SpriteDefinition.h"

MapTile::MapTile (Map& map, int col, int row, const EntityType &type) :
		IEntity(type, map), _col(col), _row(row)
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
