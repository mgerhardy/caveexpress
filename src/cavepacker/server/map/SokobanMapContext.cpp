#include "SokobanMapContext.h"
#include "common/FileSystem.h"
#include "common/Log.h"
#include "common/SpriteDefinition.h"
#include "cavepacker/shared/CavePackerEntityType.h"
#include "cavepacker/shared/CavePackerSpriteType.h"
#include "cavepacker/shared/WallTilePlacement.h"
#include <map>
#include <cstdlib>

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

bool SokobanMapContext::isWallAt(int col, int row) const {
	for (const MapTileDefinition& tileDef : _definitions) {
		if (tileDef.x != col || tileDef.y != row)
			continue;
		if (SpriteTypes::isSolid(tileDef.spriteDef->type))
			return true;
	}
	return false;
}

bool SokobanMapContext::isPlayableAt(int col, int row) const {
	if (isWallAt(col, row))
		return false;
	for (const MapTileDefinition& tileDef : _definitions) {
		if (tileDef.x != col || tileDef.y != row)
			continue;
		// ground, target, package - anything non-solid on the board
		if (!SpriteTypes::isSolid(tileDef.spriteDef->type))
			return true;
	}
	for (const IMap::StartPosition& pos : _startPositions) {
		if (string::toInt(pos._x) == col && string::toInt(pos._y) == row)
			return true;
	}
	return false;
}

void SokobanMapContext::resolveWallTiles() {
	std::map<std::string, std::vector<std::string>> rocksByPlacement;
	for (SpriteDefMapConstIter i = SpriteDefinition::get().begin(); i != SpriteDefinition::get().end(); ++i) {
		if (!string::startsWith(i->first, "tile-rock-"))
			continue;
		if (!SpriteTypes::isSolid(i->second->type))
			continue;
		const WallPlacement placement = wallPlacementFromString(i->second->placement);
		rocksByPlacement[wallPlacementToString(placement)].push_back(i->first);
	}

	const std::vector<std::string>& anyRocks = rocksByPlacement["any"];
	if (anyRocks.empty() && rocksByPlacement.empty()) {
		Log::error(LOG_GAMEIMPL, "no tile-rock sprites available for wall placement");
		return;
	}

	for (std::vector<MapTileDefinition>::iterator i = _definitions.begin(); i != _definitions.end(); ++i) {
		if (!SpriteTypes::isSolid(i->spriteDef->type))
			continue;

		const int col = static_cast<int>(i->x);
		const int row = static_cast<int>(i->y);
		const bool openL = isPlayableAt(col - 1, row);
		const bool openR = isPlayableAt(col + 1, row);
		const bool openT = isPlayableAt(col, row - 1);
		const bool openD = isPlayableAt(col, row + 1);
		const bool wallL = isWallAt(col - 1, row);
		const bool wallR = isWallAt(col + 1, row);
		const bool wallT = isWallAt(col, row - 1);
		const bool wallD = isWallAt(col, row + 1);

		const WallPlacement needed = computeWallPlacement(openL, openR, openT, openD, wallL, wallR, wallT, wallD);
		const char* neededStr = wallPlacementToString(needed);

		// Prefer orientation-specific art when available (do not dilute with "any"),
		// so rare feature tiles like cave/torch actually show up on matching edges.
		std::vector<std::string> candidates;
		if (needed != WallPlacement::Any) {
			const std::vector<std::string>& specific = rocksByPlacement[neededStr];
			candidates.insert(candidates.end(), specific.begin(), specific.end());
		}
		if (candidates.empty())
			candidates.insert(candidates.end(), anyRocks.begin(), anyRocks.end());
		if (candidates.empty()) {
			for (const auto& entry : rocksByPlacement)
				candidates.insert(candidates.end(), entry.second.begin(), entry.second.end());
		}
		if (candidates.empty())
			continue;

		const std::string& tile = candidates[rand() % candidates.size()];
		const SpriteDefPtr& spriteDefPtr = SpriteDefinition::get().getSpriteDefinition(tile);
		if (!spriteDefPtr) {
			Log::error(LOG_GAMEIMPL, "could not resolve wall tile: %s", tile.c_str());
			continue;
		}
		i->spriteDef = spriteDefPtr;
		i->angle = spriteDefPtr->angle;
	}
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
		Log::debug(LOG_GAMEIMPL, "field: %c at index %i", field, index);
		board[index] = field;
	}
	for (const EmitterDefinition& i : _emitters) {
		const int index = getIndex(i.x, i.y, width);
		char field = Sokoban::GROUND;
		const EntityType& type = *i.type;
		if (EntityTypes::isGround(type)) {
			field = Sokoban::GROUND;
		} else if (EntityTypes::isSolid(type)) {
			field = Sokoban::WALL;
		} else if (EntityTypes::isPackage(type)) {
			field = Sokoban::PACKAGE;
		} else if (EntityTypes::isTarget(type)) {
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

	if (file->length() <= 0L) {
		FS.deleteFile(path);
		return false;
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
	if (!buffer) {
		Log::error(LOG_GAMEIMPL, "Sokoban map file %s can't get opened", filePtr->getName().c_str());
		return false;
	}
	if (fileLen <= 0) {
		Log::error(LOG_GAMEIMPL, "Sokoban map file %s is empty", filePtr->getName().c_str());
		return false;
	}

	int col = 0;
	int row = 0;
	int maxCol = 0;
	bool empty = true;
	bool inComment = false;
	bool inText = false;
	// Spaces after the last non-space on a line are exterior void, not floor.
	// Defer ground placement until more content appears on the same line.
	int pendingGroundCol = -1;
	int pendingGroundCount = 0;
	std::string line;

	auto discardPendingGrounds = [&] () {
		pendingGroundCol = -1;
		pendingGroundCount = 0;
	};
	auto flushPendingGrounds = [&] () {
		for (int g = 0; g < pendingGroundCount; ++g) {
			const int groundCol = pendingGroundCol + g;
			if (!isEmpty(groundCol, row - 1))
				addGround(groundCol, row);
		}
		discardPendingGrounds();
	};

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
			flushPendingGrounds();
			addWall(col, row);
			empty = false;
			break;
		case Sokoban::GROUND:
			if (!empty) {
				if (pendingGroundCount == 0)
					pendingGroundCol = col;
				++pendingGroundCount;
			}
			break;
		case Sokoban::PLAYER:
			flushPendingGrounds();
			addGround(col, row);
			addPlayer(col, row);
			empty = false;
			break;
		case Sokoban::PACKAGE:
			flushPendingGrounds();
			addGround(col, row);
			addPackage(col, row);
			empty = false;
			break;
		case Sokoban::TARGET:
			flushPendingGrounds();
			addTarget(col, row);
			empty = false;
			break;
		case Sokoban::PACKAGEONTARGET:
			flushPendingGrounds();
			addTarget(col, row);
			addPackage(col, row);
			empty = false;
			break;
		case Sokoban::PLAYERONTARGET:
			flushPendingGrounds();
			addTarget(col, row);
			addPlayer(col, row);
			empty = false;
			break;
		case '\n':
			discardPendingGrounds();
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
	discardPendingGrounds();

	if (buffer[fileLen - 1] != '\n')
		++row;

	_settings[msn::WIDTH] = string::toString(maxCol);
	_settings[msn::HEIGHT] = string::toString(row);

	resolveWallTiles();

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
	// Provisional sprite; resolveWallTiles() selects by placement after the board is known.
	addTile("tile-rock-01", col, row);
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
