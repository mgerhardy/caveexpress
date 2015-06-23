#include "SokobanMapContext.h"
#include "common/FileSystem.h"
#include "common/Log.h"
#include "cavepacker/shared/CavePackerEntityType.h"

namespace cavepacker {

SokobanMapContext::SokobanMapContext(const std::string& map) :
		IMapContext(map), _playerSpawned(false) {
	_title = map;
}

SokobanMapContext::~SokobanMapContext() {
}

void SokobanMapContext::onMapLoaded() {
}

bool SokobanMapContext::isEmpty(int col, int row) const {
	if (row < 0)
		return false;

	for (std::vector<MapTileDefinition>::const_iterator i = _definitions.begin(); i != _definitions.end(); ++i) {
		const MapTileDefinition& tileDef = *i;
		if (tileDef.x != col)
			continue;
		if (tileDef.y != row)
			continue;
		return false;
	}
	return true;
}

bool SokobanMapContext::load(bool skipErrors) {
	_playerSpawned = false;
	Log::info(LOG_SERVER, "load the map " + _name);
	resetTiles();

	FilePtr filePtr = FS.getFile(FS.getMapsDir() + _name + ".sok");
	if (!filePtr->exists()) {
		Log::error(LOG_SERVER, "Sokoban map file " + filePtr->getName() + " does not exist");
		return false;
	}

	char *buffer;
	const int fileLen = filePtr->read((void **) &buffer);
	ScopedArrayPtr<char> p(buffer);
	if (!buffer || fileLen <= 0) {
		Log::error(LOG_SERVER, "Sokoban map file " + filePtr->getName());
		return false;
	}

	int col = 0;
	int row = 0;
	int maxCol = 0;
	bool empty = true;
	bool inComment = false;
	bool inText = false;
	std::string line;
	for (int i = 0; i < fileLen; ++i) {
		if (inText) {
			inText = buffer[i] != '\n';
			if (inText) {
				line.push_back(buffer[i]);
			} else {
				if (string::startsWith(string::toLower(line), "title:")) {
					std::vector<std::string> tokens;
					string::splitString(string::trim(line), tokens, ":");
					if (tokens.size() == 2) {
						_title = tokens[1];
					}
				}
				line = "";
			}
			continue;
		}
		if (inComment) {
			inComment = buffer[i] != '\n';
			if (inComment)
				line.push_back(buffer[i]);
			continue;
		}
		switch (buffer[i]) {
		case Sokoban::WALL:
			addWall(col, row);
			empty = false;
			break;
		case Sokoban::GROUND:
			if (!empty && !isEmpty(col, row - 1))
				addGround(col, row);
			break;
		case Sokoban::PLAYER:
			addGround(col, row);
			addPlayer(col, row);
			break;
		case Sokoban::PACKAGE:
			addGround(col, row);
			addPackage(col, row);
			break;
		case Sokoban::TARGET:
			addTarget(col, row);
			break;
		case Sokoban::PACKAGEONTARGET:
			addTarget(col, row);
			addPackage(col, row);
			break;
		case Sokoban::PLAYERONTARGET:
			addTarget(col, row);
			addPlayer(col, row);
			break;
		case '\n':
			col = -1;
			++row;
			empty = true;
			break;
		case '\r':
			continue;
		case ';':
			inComment = true;
			continue;
		default:
			inText = true;
			line.push_back(buffer[i]);
			continue;
		}
		++col;
		maxCol = std::max(maxCol, col);
	}

	if (buffer[fileLen - 1] != '\n')
		++row;

	_settings[msn::WIDTH] = string::toString(maxCol);
	_settings[msn::HEIGHT] = string::toString(row);

	Log::info(LOG_SERVER, "found " + string::toString(_startPositions.size()) + " start positions");

	return _playerSpawned;
}

void SokobanMapContext::addTile(const std::string& tile, int col, int row) {
	const SpriteDefPtr &spriteDefPtr = SpriteDefinition::get().getSpriteDefinition(
			tile);
	if (!spriteDefPtr) {
		Log::error(LOG_SERVER, "could not add tile: " + tile);
		return;
	}

	const EntityAngle angle = spriteDefPtr->angle;
	const MapTileDefinition def(col, row, spriteDefPtr, angle);
	_definitions.push_back(def);
}

inline void SokobanMapContext::addTarget(int col, int row) {
	addTile("target", col, row);
}

inline void SokobanMapContext::addWall(int col, int row) {
	const int rnd = rand() % 3 + 1;
	addTile("tile-rock-" + String::format("%02i", rnd), col, row);
}

inline void SokobanMapContext::addPackage(int col, int row) {
	addTile("package", col, row);
}

inline void SokobanMapContext::addPlayer(int col, int row) {
	const IMap::StartPosition p{string::toString(col), string::toString(row)};
	_startPositions.push_back(p);
	_playerSpawned = true;
}

inline void SokobanMapContext::addGround(int col, int row) {
	const int rnd = rand() % 4 + 1;
	addTile("tile-background-" + String::format("%02i", rnd), col, row);
}

}
