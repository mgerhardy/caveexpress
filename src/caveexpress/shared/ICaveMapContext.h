#pragma once

#include <vector>
#include "common/IMapContext.h"
#include "CaveTileDefinition.h"

namespace caveexpress {

class ICaveMapContext: public IMapContext {
protected:
	std::vector<CaveTileDefinition> _caveDefinitions;

	ICaveMapContext(const std::string& name) :
			IMapContext(name) {
	}
public:
	inline const std::vector<CaveTileDefinition>& getCaveTileDefinitions() const {
		return _caveDefinitions;
	}
};

}
