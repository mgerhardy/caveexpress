#pragma once

#include "caveexpress/server/entities/IEntity.h"
#include "common/IMap.h"
#include "common/SpriteDefinition.h" /* TODO: just because of: typedef int16_t EntityAngle; ?? */

namespace caveexpress {

// forward decl
class Map;


class MapTile: public IEntity {
protected:
	gridCoord _gridX;
	gridCoord _gridY;
	gridSize _gridWidth;
	gridSize _gridHeight;
	EntityAngle _angle;

	b2Vec2 _pos;

public:
	MapTile (Map& map, const std::string& spriteID, gridCoord gridX, gridCoord gridY, const EntityType &type);
	virtual ~MapTile ();

	gridCoord getGridX () const;
	gridCoord getGridY () const;
	gridSize getGridWidth () const;
	gridSize getGridHeight () const;

	void setGridDimensions (gridSize gridWidth, gridSize gridHeight, EntityAngle angle);

	virtual void createBody ();

	bool operator< (const MapTile& other);
	bool operator< (const MapTile* other);

	// IEntity
	bool shouldCollide (const IEntity* entity) const override;
	const b2Vec2& getPos () const override;
	SpriteDefPtr getSpriteDef () const override;
};

inline void MapTile::setGridDimensions (float gridWidth, float gridHeight, EntityAngle angle)
{
	_size = b2Vec2(gridWidth, gridHeight);
	_gridWidth = gridWidth;
	_gridHeight = gridHeight;
	_angle = angle;
	_pos = b2Vec2(_gridX + gridWidth / 2.0f, _gridY + gridHeight / 2.0f);
}

inline gridCoord MapTile::getGridX () const
{
	return _gridX;
}

inline gridCoord MapTile::getGridY () const
{
	return _gridY;
}

inline gridSize MapTile::getGridWidth () const
{
	return _gridWidth;
}

inline gridSize MapTile::getGridHeight () const
{
	return _gridHeight;
}

}
