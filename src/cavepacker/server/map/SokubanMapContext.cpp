#include "SokubanMapContext.h"
#include "engine/common/FileSystem.h"
#include "engine/common/Logger.h"
#include "cavepacker/shared/CavePackerEntityType.h"

SokubanMapContext::SokubanMapContext(const std::string& map) :
		IMapContext(map), _playerSpawned(false) {
	_title = map;
}

SokubanMapContext::~SokubanMapContext() {
}

void SokubanMapContext::onMapLoaded() {
}

bool SokubanMapContext::isEmpty(int col, int row) const {
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

bool SokubanMapContext::load(bool skipErrors) {
	info(LOG_SERVER, "load the map " + _name);
	resetTiles();

	FilePtr filePtr = FS.getFile(FS.getMapsDir() + _name + ".sok");
	if (!filePtr->exists()) {
		error(LOG_SERVER,
				"sokuban map file " + filePtr->getURI().print()
						+ " does not exist");
		return false;
	}

	char *buffer;
	const int fileLen = filePtr->read((void **) &buffer);
	ScopedArrayPtr<char> p(buffer);
	if (!buffer || fileLen <= 0) {
		error(LOG_SERVER, "sokuban map file " + filePtr->getURI().print());
		return false;
	}

	int col = 0;
	int row = 0;
	int maxCol = 0;
	bool empty = true;
	bool inComment = false;
	for (int i = 0; i < fileLen; ++i) {
		if (inComment) {
			inComment = buffer[i] == '\n';
			if (inComment)
				continue;
		}
		switch (buffer[i]) {
		case Sokuban::WALL:
			addWall(col, row);
			empty = false;
			break;
		case Sokuban::GROUND:
			if (!empty && !isEmpty(col, row - 1))
				addGround(col, row);
			break;
		case Sokuban::PLAYER:
			addGround(col, row);
			addPlayer(col, row);
			break;
		case Sokuban::PACKAGE:
			addGround(col, row);
			addPackage(col, row);
			break;
		case Sokuban::TARGET:
			addTarget(col, row);
			break;
		case Sokuban::PACKAGEONTARGET:
			addTarget(col, row);
			addPackage(col, row);
			break;
		case Sokuban::PLAYERONTARGET:
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
		default:
			inComment = true;
			continue;
		}
		++col;
		maxCol = std::max(maxCol, col);
	}

	_settings[msn::WIDTH] = string::toString(maxCol);
	_settings[msn::HEIGHT] = string::toString(row);

	return _playerSpawned;
}

void SokubanMapContext::addTile(const std::string& tile, int col, int row) {
	const SpriteDefPtr &spriteDefPtr = SpriteDefinition::get().getSpriteDefinition(
			tile);
	if (!spriteDefPtr) {
		error(LOG_SERVER, "could not add tile: " + tile);
		return;
	}

	const EntityAngle angle = spriteDefPtr->angle;
	const MapTileDefinition def(col, row, spriteDefPtr, angle);
	_definitions.push_back(def);
}

inline void SokubanMapContext::addTarget(int col, int row) {
	addTile("target", col, row);
}

inline void SokubanMapContext::addWall(int col, int row) {
	const int rnd = rand() % 3 + 1;
	addTile("tile-rock-" + String::format("%02i", rnd), col, row);
}

inline void SokubanMapContext::addPackage(int col, int row) {
	addTile("package", col, row);
}

inline void SokubanMapContext::addPlayer(int col, int row) {
	_settings[msn::PLAYER_X] = string::toString(col);
	_settings[msn::PLAYER_Y] = string::toString(row);
	_playerSpawned = true;
}

inline void SokubanMapContext::addGround(int col, int row) {
	const int rnd = rand() % 4 + 1;
	addTile("tile-background-" + String::format("%02i", rnd), col, row);
}
