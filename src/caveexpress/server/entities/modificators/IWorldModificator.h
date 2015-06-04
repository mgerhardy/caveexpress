#pragma once

#include "caveexpress/server/entities/IEntity.h"

class Map;

class IWorldModificator: public IEntity {
public:
	explicit IWorldModificator (Map& map) :
			IEntity(EntityTypes::MODIFICATOR, map)
	{
	}

	virtual ~IWorldModificator ()
	{
	}

	virtual void setModificatorState (bool enable) = 0;
	virtual void setRelativePositionTo (const b2Vec2& pos) = 0;
};
