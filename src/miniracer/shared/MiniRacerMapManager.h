#pragma once

#include "miniracer/server/map/MiniRacerMapContext.h"
#include "common/FileSystem.h"

namespace miniracer {

class MiniRacerMapManager: public IMapManager {
public:
	MiniRacerMapManager() :
			IMapManager("map") {
	}

	int getStartPositions (const std::string& filename) override {
		// TODO
		return 1;
	}
};

}
