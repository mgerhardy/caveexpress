#pragma once

#include "common/LUAMapContext.h"
#include "miniracer/shared/MiniRacerSpriteType.h"

namespace miniracer {

class MiniRacerMapContext: public LUAMapContext {
protected:
	bool isSolid (const SpriteType& type) const {
		return SpriteTypes::isSolid(type);
	}
public:
	explicit MiniRacerMapContext (const std::string& name) :
			LUAMapContext(name) {
	}
	virtual ~MiniRacerMapContext () {}
};

}
