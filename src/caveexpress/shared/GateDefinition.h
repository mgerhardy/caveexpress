#pragma once

#include "common/MapTileDefinition.h"
#include <string>

namespace caveexpress {

class GateDefinition: public MapTileDefinition {
public:
	std::string linkId;
	float openAmount;

	GateDefinition (gridCoord _x, gridCoord _y, const SpriteDefPtr &_spriteDef, const std::string& _linkId,
			float _openAmount) :
			MapTileDefinition(_x, _y, _spriteDef, 0), linkId(_linkId), openAmount(_openAmount)
	{
	}
};

}
