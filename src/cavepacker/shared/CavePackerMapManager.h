#pragma once

#include "cavepacker/server/map/SokobanMapContext.h"
#include "common/FileSystem.h"

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
		int players = 0;
		for (int i = 0; i < fileLen; ++i) {
			if (buffer[i] == Sokoban::PLAYER || buffer[i] == Sokoban::PLAYERONTARGET) {
				++players;
			}
		}
		return players;
	}
};

}
