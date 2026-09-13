#pragma once

#include "cavepacker/server/map/SokobanMapContext.h"
#include "common/FileSystem.h"
#include "common/MapManager.h"
#include "common/StartPositionMode.h"
#include <memory>

namespace cavepacker {

class CavePackerMapManager: public IMapManager {
public:
	CavePackerMapManager() :
			IMapManager("sok") {
	}

	int getStartPositions (const std::string& filename) override {
		const FilePtr& file = FS.getFile(filename);
		char *buffer;
		const int fileLen = file->read((void **) &buffer);
		const std::unique_ptr<char[]> p(buffer);
		if (!buffer || fileLen <= 0)
			return 0;
		return StartPositionModes::countMultiplayerStartsFromSokoban(std::string(buffer, fileLen));
	}
};

}
