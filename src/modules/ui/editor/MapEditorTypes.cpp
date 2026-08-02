#include "ui/editor/MapEditorTypes.h"
#include "common/vec2.h"
#include <cmath>

bool MapEditorTileItem::operator== (const MapEditorTileItem& other) const
{
	return gridX == other.gridX && gridY == other.gridY && layer == other.layer
			&& def.get() == other.def.get() && entityType == other.entityType;
}

bool MapEditorTileItem::operator< (const MapEditorTileItem& other) const
{
	if (gridX != other.gridX)
		return gridX < other.gridX;
	if (gridY != other.gridY)
		return gridY < other.gridY;
	return false;
}

gridCoord MapEditorTileItem::getX (bool useShape) const
{
	if (useShape && def->hasShape())
		return def->getMins().x;
	const vec2 size = getSize();
	if (size.x <= 1.0f + EPSILON)
		return 0.0f;
	float integralPart;
	return -::modff(size.x, &integralPart);
}

gridCoord MapEditorTileItem::getY (bool useShape) const
{
	if (mapTile)
		return 0.0f;
	if (useShape && def->hasShape())
		return def->getMins().y;
	const vec2 size = getSize();
	if (size.y <= 1.0f + EPSILON)
		return 1.0f - size.y;
	float integralPart;
	return -::modff(size.y, &integralPart);
}

vec2 MapEditorTileItem::getSize (bool useShape) const
{
	if (useShape && def->hasShape())
		return def->calculateSizeFromShapeData();
	if (mapTile)
		return vec2(def->width, def->height);
	if (entityType)
		return vec2(entityType->width, entityType->height);
	return vec2(def->width, def->height);
}
