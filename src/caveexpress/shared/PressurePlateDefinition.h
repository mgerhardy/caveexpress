#pragma once

#include "common/MapTileDefinition.h"
#include <string>

namespace caveexpress {

class PressurePlateDefinition: public MapTileDefinition {
public:
	std::string linkId;
	float requiredWeight;
	int holdMs;

	PressurePlateDefinition (gridCoord _x, gridCoord _y, const SpriteDefPtr &_spriteDef, const std::string& _linkId,
			float _requiredWeight, int _holdMs) :
			MapTileDefinition(_x, _y, _spriteDef, 0), linkId(_linkId), requiredWeight(_requiredWeight), holdMs(_holdMs)
	{
	}
};

}
