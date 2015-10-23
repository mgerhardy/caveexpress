#include "SokobanMapContext.h"
#include "common/FileSystem.h"
#include "common/Log.h"
#include "cavepacker/shared/CavePackerEntityType.h"
#include "cavepacker/shared/CavePackerSpriteType.h"

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

static inline int getIndex(int col, int row, int width) {
	return col + width * row;
}

bool SokobanMapContext::save () const {
	const IMap::SettingsMap& settings = _settings;
	const auto widthIter = settings.find(msn::WIDTH);
	if (widthIter == settings.end()) {
		return false;
	}
	const auto heightIter = settings.find(msn::HEIGHT);
	if (heightIter == settings.end()) {
		return false;
	}

	const std::string path = FS.getAbsoluteWritePath() + FS.getDataDir() + FS.getMapsDir() + _name + ".sok";
	SDL_RWops *rwops = FS.createRWops(path, "wb");
	FilePtr file(new File(rwops, path));

	const int width = string::toInt(widthIter->second);
	const int height = string::toInt(heightIter->second);

	file->writeString(";");
	file->appendString(_name.c_str());
	file->appendString("\n");

	std::vector<char> board;
	board.resize(width * height, Sokoban::GROUND);

	for (const MapTileDefinition& i : _definitions) {
		const int index = getIndex(i.x, i.y, width);
		char field = Sokoban::GROUND;
		const SpriteType& spriteType = i.spriteDef->type;
		if (SpriteTypes::isSolid(spriteType)) {
			field = Sokoban::WALL;
		} else if (SpriteTypes::isTarget(spriteType)) {
			field = Sokoban::TARGET;
		} else if (SpriteTypes::isPackage(spriteType)) {
			field = Sokoban::PACKAGE;
		}
		if (board[index] == Sokoban::TARGET) {
			if (field == Sokoban::PLAYER)
				field = Sokoban::PLAYERONTARGET;
			else if (field == Sokoban::PACKAGE)
				field = Sokoban::PACKAGEONTARGET;
		}
		Log::error(LOG_GAMEIMPL, "field: %c at index %i", field, index);
		board[index] = field;
	}
	for (const EmitterDefinition& i : _emitters) {
		const int index = getIndex(i.x, i.y, width);
		char field = Sokoban::GROUND;
		if (EntityTypes::isGround(*i.type)) {
			field = Sokoban::GROUND;
		} else if (EntityTypes::isSolid(*i.type)) {
			field = Sokoban::WALL;
		} else if (EntityTypes::isPackage(*i.type)) {
			field = Sokoban::PACKAGE;
		} else if (EntityTypes::isTarget(*i.type)) {
			field = Sokoban::TARGET;
		}
		if (isTarget(board[index] && isPackage(field))) {
			field = Sokoban::PACKAGEONTARGET;
		}
		board[index] = field;
	}

	for (const IMap::StartPosition& pos : _startPositions) {
		const int x = string::toInt(pos._x);
		const int y = string::toInt(pos._y);
		const int index = getIndex(x, y, width);
		if (isTarget(board[index])) {
			board[index] = Sokoban::PLAYERONTARGET;
		} else {
			board[index] = Sokoban::PLAYER;
		}
	}

	int col = 0;
	for (int i = 0; i < width * height; ++i) {
		const char field = board[i];
		char str[2] = { field, '\0' };
		file->appendString(str);
		++col;
		if (col >= width) {
			file->appendString("\n");
			col = 0;
		}
	}

	return true;
}

bool SokobanMapContext::load(bool skipErrors) {
	_playerSpawned = false;
	Log::info(LOG_GAMEIMPL, "load the map %s", _name.c_str());
	resetTiles();

	FilePtr filePtr = FS.getFileFromURL("maps://" + _name + ".sok");
	if (!filePtr->exists()) {
		Log::error(LOG_GAMEIMPL, "Sokoban map file '%s' does not exist", filePtr->getName().c_str());
		return false;
	}

	char *buffer;
	const int fileLen = filePtr->read((void **) &buffer);
	std::unique_ptr<char[]> p(buffer);
	if (!buffer || fileLen <= 0) {
		Log::error(LOG_GAMEIMPL, "Sokoban map file %s", filePtr->getName().c_str());
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
				const std::string& lower = string::toLower(line);
				if (string::startsWith(lower, "title:") || string::startsWith(lower, "collection:")) {
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
			else
				Log::info(LOG_GAMEIMPL, "comment: %s", line.c_str());
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
		case ':':
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

	Log::info(LOG_GAMEIMPL, "found %i start positions", (int)_startPositions.size());

	return _playerSpawned;
}

void SokobanMapContext::addTile(const std::string& tile, int col, int row) {
	const SpriteDefPtr &spriteDefPtr = SpriteDefinition::get().getSpriteDefinition(
			tile);
	if (!spriteDefPtr) {
		Log::error(LOG_GAMEIMPL, "could not add tile: %s", tile.c_str());
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
	addTile("tile-rock-" + string::format("%02i", rnd), col, row);
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
	addTile("tile-background-" + string::format("%02i", rnd), col, row);
}

}
