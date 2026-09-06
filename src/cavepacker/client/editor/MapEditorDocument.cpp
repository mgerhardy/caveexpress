#include "MapEditorDocument.h"
#include "cavepacker/server/map/SokobanMapContext.h"
#include "cavepacker/shared/CavePackerSpriteType.h"
#include "cavepacker/shared/CavePackerEntityType.h"
#include "cavepacker/shared/WallTilePlacement.h"
#include "common/FileSystem.h"
#include "common/Log.h"
#include "common/String.h"
#include "common/Math.h"
#include "common/MapSettings.h"
#include "cavepacker/shared/SokobanTiles.h"
#include <algorithm>
#include <cmath>
#include <cstring>
#include <map>
#include <queue>
#include <cstdlib>

namespace cavepacker {

namespace {

bool campaignPatternMatches (const std::string& pattern, const std::string& mapId)
{
	if (pattern == mapId)
		return true;
	if (!pattern.empty() && pattern.back() == '*')
		return string::startsWith(mapId, pattern.substr(0, pattern.size() - 1));
	return false;
}

std::string readCampaignSource (const std::string& campaignFile)
{
	const std::string rel = FS.getCampaignsDir() + campaignFile;
	FilePtr file = FS.getFile(rel);
	if (!file || !file->exists())
		return "";
	void* buf = nullptr;
	const int n = file->read(&buf);
	if (n <= 0 || buf == nullptr) {
		delete[] static_cast<char*>(buf);
		return "";
	}
	std::string source(static_cast<char*>(buf), static_cast<size_t>(n));
	delete[] static_cast<char*>(buf);
	return source;
}

bool writeCampaignSource (const std::string& campaignFile, const std::string& source)
{
	const std::string rel = FS.getCampaignsDir() + campaignFile;
	const std::string path = FS.getDataDir() + rel;
	return FS.writeSysFile(path, reinterpret_cast<const unsigned char*>(source.c_str()), source.size(), true) >= 0;
}

void collectCampaignMapIds (const std::string& source, std::vector<std::string>& out)
{
	const char* keys[] = { "c:addMaps(\"", "c:addMap(\"" };
	for (const char* key : keys) {
		size_t pos = 0;
		const size_t keyLen = std::strlen(key);
		while ((pos = source.find(key, pos)) != std::string::npos) {
			pos += keyLen;
			const size_t end = source.find('"', pos);
			if (end == std::string::npos)
				break;
			out.push_back(source.substr(pos, end - pos));
			pos = end + 1;
		}
	}
}

std::string campaignAddLine (const std::string& mapId)
{
	return "c:addMaps(\"" + mapId + "\")";
}

}

MapEditorDocument::MapEditorDocument (IMapManager& mapManager) :
		IMapEditorDocument(mapManager)
{
}

MapEditorLayer MapEditorDocument::getLayer (const SpriteType& type) const
{
	if (SpriteTypes::isSolid(type))
		return LAYER_SOLID;
	if (SpriteTypes::isPackage(type))
		return LAYER_EMITTER;
	return LAYER_BACKGROUND;
}

bool MapEditorDocument::isMapTileType (const SpriteType& type) const
{
	return SpriteTypes::isMapTile(type);
}

bool MapEditorDocument::isPlayerType (const EntityType& type) const
{
	return EntityTypes::isPlayer(type);
}

const Animation& MapEditorDocument::getEmitterAnimation (const EntityType& type) const
{
	return Animation::NONE;
}

const EntityType& MapEditorDocument::getPlayerEntityType () const
{
	return EntityTypes::PLAYER;
}

std::unique_ptr<IMapContext> MapEditorDocument::createContext (const std::string& mapName) const
{
	return std::unique_ptr<IMapContext>(new SokobanMapContext(mapName));
}

std::string MapEditorDocument::getUserMapsPath () const
{
	return FS.getAbsoluteWritePath() + FS.getDataDir() + FS.getMapsDir() + _fileName + ".sok";
}

std::string MapEditorDocument::getGameDataMapsPath () const
{
	return FS.getDataDir() + FS.getMapsDir() + _fileName + ".sok";
}

bool MapEditorDocument::isWallItem (const MapEditorTileItem& item) const
{
	return item.def && SpriteTypes::isSolid(item.def->type);
}

bool MapEditorDocument::isPackageItem (const MapEditorTileItem& item) const
{
	if (item.entityType != nullptr && EntityTypes::isPackage(*item.entityType))
		return true;
	return item.def && SpriteTypes::isPackage(item.def->type);
}

bool MapEditorDocument::isTargetItem (const MapEditorTileItem& item) const
{
	return item.def && SpriteTypes::isTarget(item.def->type);
}

bool MapEditorDocument::isGroundItem (const MapEditorTileItem& item) const
{
	return item.def && SpriteTypes::isGround(item.def->type);
}

bool MapEditorDocument::sameCell (const MapEditorTileItem& a, const MapEditorTileItem& b) const
{
	return static_cast<int>(std::floor(a.gridX + EPSILON)) == static_cast<int>(std::floor(b.gridX + EPSILON))
			&& static_cast<int>(std::floor(a.gridY + EPSILON)) == static_cast<int>(std::floor(b.gridY + EPSILON));
}

bool MapEditorDocument::isWallAt (int col, int row) const
{
	for (const MapEditorTileItem& item : _map) {
		if (!isWallItem(item))
			continue;
		if (static_cast<int>(std::floor(item.gridX + EPSILON)) != col)
			continue;
		if (static_cast<int>(std::floor(item.gridY + EPSILON)) != row)
			continue;
		return true;
	}
	return false;
}

bool MapEditorDocument::isPlayableAt (int col, int row) const
{
	if (col < 0 || row < 0 || col >= _mapWidth || row >= _mapHeight)
		return false;
	if (isWallAt(col, row))
		return false;
	for (const MapEditorTileItem& item : _map) {
		if (isWallItem(item))
			continue;
		if (static_cast<int>(std::floor(item.gridX + EPSILON)) != col)
			continue;
		if (static_cast<int>(std::floor(item.gridY + EPSILON)) != row)
			continue;
		return true;
	}
	for (const IMap::StartPosition& pos : _startPositions) {
		if (string::toInt(pos._x) == col && string::toInt(pos._y) == row)
			return true;
	}
	return false;
}

bool MapEditorDocument::isOverlapping (const MapEditorTileItem& item1, const MapEditorTileItem& item2) const
{
	if (!sameCell(item1, item2))
		return false;
	if (isWallItem(item1) || isWallItem(item2))
		return true;
	if (isPackageItem(item1) && isPackageItem(item2))
		return true;
	if ((isGroundItem(item1) || isTargetItem(item1)) && (isGroundItem(item2) || isTargetItem(item2)))
		return true;
	return false;
}

bool MapEditorDocument::canPlaceTileItem (const MapEditorTileItem& item) const
{
	const int col = static_cast<int>(std::floor(item.gridX + EPSILON));
	const int row = static_cast<int>(std::floor(item.gridY + EPSILON));
	if (col < 0 || row < 0 || col >= _mapWidth || row >= _mapHeight)
		return false;
	if (isPackageItem(item) && isWallAt(col, row))
		return false;
	if ((isGroundItem(item) || isTargetItem(item)) && isWallAt(col, row))
		return false;
	return true;
}

bool MapEditorDocument::floodFillCanPaint (int x, int y, const SpriteDefPtr& brush) const
{
	if (!brush)
		return false;
	if (isWallAt(x, y) && !SpriteTypes::isSolid(brush->type))
		return false;
	return true;
}

bool MapEditorDocument::shouldSaveTile (const MapEditorTileItem& tile) const
{
	return tile.entityType == nullptr || isPackageItem(tile);
}

bool MapEditorDocument::shouldSaveEmitter (const MapEditorTileItem& tile) const
{
	return tile.entityType != nullptr && !isPackageItem(tile);
}

void MapEditorDocument::fillTilePalette (std::vector<SpriteDefPtr>& out) const
{
	for (SpriteDefMapConstIter i = SpriteDefinition::get().begin(); i != SpriteDefinition::get().end(); ++i) {
		const SpriteDefPtr& sprite = i->second;
		if (!SpriteTypes::isMapTile(sprite->type))
			continue;
		if (sprite->hasNoTextures())
			continue;
		out.push_back(sprite);
	}
}

void MapEditorDocument::fillEntityPalette (std::vector<const EntityType*>& out) const
{
	out.push_back(&EntityTypes::PLAYER);
	out.push_back(&EntityTypes::PACKAGE);
}

void MapEditorDocument::tagPackageItems ()
{
	for (MapEditorTileItem& item : _map) {
		if (!item.def || !SpriteTypes::isPackage(item.def->type))
			continue;
		item.entityType = &EntityTypes::PACKAGE;
		item.layer = LAYER_EMITTER;
		item.mapTile = true;
	}
}

void MapEditorDocument::loadFromContext (IMapContext& ctx)
{
	IMapEditorDocument::loadFromContext(ctx);
	tagPackageItems();
}

void MapEditorDocument::onAfterStateRestored ()
{
	tagPackageItems();
}

bool MapEditorDocument::placeBrushItem (bool overwrite)
{
	if (!_activeSprite)
		return false;
	if (_activeEntityType != nullptr && isPlayerType(*_activeEntityType)) {
		const int col = static_cast<int>(std::floor(_selectedGridX + EPSILON));
		const int row = static_cast<int>(std::floor(_selectedGridY + EPSILON));
		if (isWallAt(col, row))
			return false;
		setPlayerPosition(_selectedGridX, _selectedGridY);
		autoTileWalls();
		return true;
	}

	MapEditorTileItem item;
	item.gridX = _selectedGridX;
	item.gridY = _selectedGridY;
	item.angle = _activeAngle;
	if (_activeEntityType != nullptr && EntityTypes::isPackage(*_activeEntityType)) {
		item.def = SpriteDefinition::get().getSpriteDefinition("package");
		if (!item.def)
			item.def = _activeSprite;
		item.entityType = &EntityTypes::PACKAGE;
		item.layer = LAYER_EMITTER;
		item.mapTile = true;
	} else {
		item.def = _activeSprite;
		item.entityType = nullptr;
		item.layer = getLayer(item.def->type);
		item.mapTile = isMapTileType(item.def->type);
	}
	if (!canPlaceTileItem(item))
		return false;
	if (!placeTileItem(item, overwrite))
		return false;
	autoTileWalls();
	return true;
}

bool MapEditorDocument::eraseAtSelection (bool recordUndo)
{
	const bool erased = IMapEditorDocument::eraseAtSelection(recordUndo);
	if (erased)
		autoTileWalls();
	return erased;
}

int MapEditorDocument::countWalls () const
{
	int n = 0;
	for (const MapEditorTileItem& item : _map) {
		if (isWallItem(item))
			++n;
	}
	return n;
}

int MapEditorDocument::countGrounds () const
{
	int n = 0;
	for (const MapEditorTileItem& item : _map) {
		if (isGroundItem(item))
			++n;
	}
	return n;
}

int MapEditorDocument::countPackages () const
{
	int n = 0;
	for (const MapEditorTileItem& item : _map) {
		if (isPackageItem(item))
			++n;
	}
	return n;
}

int MapEditorDocument::countTargets () const
{
	int n = 0;
	for (const MapEditorTileItem& item : _map) {
		if (isTargetItem(item))
			++n;
	}
	return n;
}

void MapEditorDocument::autoTileWalls (bool recordUndo)
{
	if (recordUndo)
		MapEditorUndo();
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
	if (anyRocks.empty() && rocksByPlacement.empty())
		return;

	for (MapEditorTileItem& item : _map) {
		if (!isWallItem(item))
			continue;
		const int col = static_cast<int>(std::floor(item.gridX + EPSILON));
		const int row = static_cast<int>(std::floor(item.gridY + EPSILON));
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
		if (item.def) {
			bool keep = false;
			for (const std::string& id : candidates) {
				if (id == item.def->id) {
					keep = true;
					break;
				}
			}
			if (keep)
				continue;
		}
		const std::string& tile = candidates[rand() % candidates.size()];
		const SpriteDefPtr def = SpriteDefinition::get().getSpriteDefinition(tile);
		if (!def)
			continue;
		item.def = def;
		item.angle = def->angle;
	}
}

bool MapEditorDocument::findOpenCell (int& x, int& y) const
{
	for (int row = 1; row < _mapHeight - 1; ++row) {
		for (int col = 1; col < _mapWidth - 1; ++col) {
			if (isWallAt(col, row))
				continue;
			bool occupied = false;
			for (const MapEditorTileItem& item : _map) {
				if (!isPackageItem(item) && !isTargetItem(item))
					continue;
				if (static_cast<int>(std::floor(item.gridX + EPSILON)) == col
						&& static_cast<int>(std::floor(item.gridY + EPSILON)) == row) {
					occupied = true;
					break;
				}
			}
			for (const IMap::StartPosition& pos : _startPositions) {
				if (string::toInt(pos._x) == col && string::toInt(pos._y) == row)
					occupied = true;
			}
			if (occupied)
				continue;
			x = col;
			y = row;
			return true;
		}
	}
	return false;
}

void MapEditorDocument::makePlayable ()
{
	MapEditorUndo();
	const SpriteDefPtr wall = SpriteDefinition::get().getSpriteDefinition("tile-rock-01");
	const SpriteDefPtr ground = SpriteDefinition::get().getSpriteDefinition("tile-background-01");
	const SpriteDefPtr target = SpriteDefinition::get().getSpriteDefinition("target");
	const SpriteDefPtr package = SpriteDefinition::get().getSpriteDefinition("package");
	if (!wall || !ground)
		return;

	if (countWalls() == 0) {
		for (int col = 0; col < _mapWidth; ++col) {
			for (int row = 0; row < _mapHeight; ++row) {
				if (col > 0 && col < _mapWidth - 1 && row > 0 && row < _mapHeight - 1)
					continue;
				MapEditorTileItem item;
				item.def = wall;
				item.gridX = static_cast<gridCoord>(col);
				item.gridY = static_cast<gridCoord>(row);
				item.layer = LAYER_SOLID;
				item.mapTile = true;
				placeTileItem(item, true);
			}
		}
	}

	for (int col = 1; col < _mapWidth - 1; ++col) {
		for (int row = 1; row < _mapHeight - 1; ++row) {
			if (isWallAt(col, row) || isPlayableAt(col, row))
				continue;
			MapEditorTileItem item;
			item.def = ground;
			item.gridX = static_cast<gridCoord>(col);
			item.gridY = static_cast<gridCoord>(row);
			item.layer = LAYER_BACKGROUND;
			item.mapTile = true;
			placeTileItem(item, false);
		}
	}

	int x = 1;
	int y = 1;
	if (countTargets() == 0 && target && findOpenCell(x, y)) {
		MapEditorTileItem item;
		item.def = target;
		item.gridX = static_cast<gridCoord>(x);
		item.gridY = static_cast<gridCoord>(y);
		item.layer = LAYER_BACKGROUND;
		item.mapTile = true;
		placeTileItem(item, true);
	}
	if (countPackages() == 0 && package && findOpenCell(x, y)) {
		MapEditorTileItem item;
		item.def = package;
		item.entityType = &EntityTypes::PACKAGE;
		item.gridX = static_cast<gridCoord>(x);
		item.gridY = static_cast<gridCoord>(y);
		item.layer = LAYER_EMITTER;
		item.mapTile = true;
		placeTileItem(item, false);
	}
	if (_startPositions.empty() && findOpenCell(x, y))
		setPlayerPosition(static_cast<gridCoord>(x), static_cast<gridCoord>(y));
	autoTileWalls();
}

bool MapEditorDocument::evaluateReachability (int& reachablePlayable, int& playableCells, std::string& failure) const
{
	reachablePlayable = 0;
	playableCells = 0;
	failure.clear();
	if (_startPositions.empty()) {
		failure = "no player start";
		return false;
	}
	const int sx = string::toInt(_startPositions.front()._x);
	const int sy = string::toInt(_startPositions.front()._y);
	if (isWallAt(sx, sy)) {
		failure = "player start is inside a wall";
		return false;
	}

	std::vector<char> seen(static_cast<size_t>(_mapWidth * _mapHeight), 0);
	std::queue<std::pair<int, int>> q;
	auto enqueue = [&] (int x, int y) {
		if (x < 0 || y < 0 || x >= _mapWidth || y >= _mapHeight)
			return;
		if (isWallAt(x, y))
			return;
		if (!isPlayableAt(x, y) && !(x == sx && y == sy))
			return;
		const size_t idx = static_cast<size_t>(y * _mapWidth + x);
		if (seen[idx])
			return;
		seen[idx] = 1;
		q.push({x, y});
	};
	enqueue(sx, sy);
	while (!q.empty()) {
		const int x = q.front().first;
		const int y = q.front().second;
		q.pop();
		if (isPlayableAt(x, y) || (x == sx && y == sy))
			++reachablePlayable;
		enqueue(x + 1, y);
		enqueue(x - 1, y);
		enqueue(x, y + 1);
		enqueue(x, y - 1);
	}

	for (int col = 0; col < _mapWidth; ++col) {
		for (int row = 0; row < _mapHeight; ++row) {
			if (isPlayableAt(col, row))
				++playableCells;
		}
	}

	auto reachableCell = [&] (int col, int row) -> bool {
		if (col < 0 || row < 0 || col >= _mapWidth || row >= _mapHeight)
			return false;
		return seen[static_cast<size_t>(row * _mapWidth + col)] != 0;
	};

	for (const MapEditorTileItem& item : _map) {
		const int col = static_cast<int>(std::floor(item.gridX + EPSILON));
		const int row = static_cast<int>(std::floor(item.gridY + EPSILON));
		if (isPackageItem(item) && !reachableCell(col, row)) {
			failure = "a package is not reachable from the player";
			return false;
		}
		if (isTargetItem(item) && !reachableCell(col, row)) {
			failure = "a target is not reachable from the player";
			return false;
		}
	}
	return true;
}

void MapEditorDocument::collectGameValidationIssues (std::vector<std::string>& out) const
{
	const int packages = countPackages();
	const int targets = countTargets();
	if (packages == 0)
		out.push_back("No packages");
	if (targets == 0)
		out.push_back("No targets");
	if (packages != targets)
		out.push_back("Package count must match target count");
	for (const IMap::StartPosition& pos : _startPositions) {
		const int x = string::toInt(pos._x);
		const int y = string::toInt(pos._y);
		if (isWallAt(x, y))
			out.push_back("Player start is inside a wall");
	}
	for (const MapEditorTileItem& item : _map) {
		if (!isPackageItem(item))
			continue;
		const int col = static_cast<int>(std::floor(item.gridX + EPSILON));
		const int row = static_cast<int>(std::floor(item.gridY + EPSILON));
		if (isWallAt(col, row))
			out.push_back("A package is on a wall");
	}
	int reachable = 0;
	int playable = 0;
	std::string failure;
	if (!evaluateReachability(reachable, playable, failure) && !failure.empty())
		out.push_back(failure);
}

char MapEditorDocument::cellGlyphAt (int col, int row) const
{
	const bool wall = isWallAt(col, row);
	bool package = false;
	bool target = false;
	for (const MapEditorTileItem& item : _map) {
		if (static_cast<int>(std::floor(item.gridX + EPSILON)) != col)
			continue;
		if (static_cast<int>(std::floor(item.gridY + EPSILON)) != row)
			continue;
		if (isPackageItem(item))
			package = true;
		if (isTargetItem(item))
			target = true;
	}
	bool player = false;
	for (const IMap::StartPosition& pos : _startPositions) {
		if (string::toInt(pos._x) == col && string::toInt(pos._y) == row)
			player = true;
	}
	if (wall)
		return Sokoban::WALL;
	if (player && target)
		return Sokoban::PLAYERONTARGET;
	if (package && target)
		return Sokoban::PACKAGEONTARGET;
	if (player)
		return Sokoban::PLAYER;
	if (package)
		return Sokoban::PACKAGE;
	if (target)
		return Sokoban::TARGET;
	if (isPlayableAt(col, row))
		return Sokoban::GROUND;
	return ' ';
}

bool MapEditorDocument::campaignContainsExact (const std::string& campaignFile) const
{
	if (_fileName.empty())
		return false;
	for (const std::string& pattern : listMapsInCampaign(campaignFile)) {
		if (pattern == _fileName)
			return true;
	}
	return false;
}

std::vector<std::string> MapEditorDocument::listCampaignFiles () const
{
	std::vector<std::string> out;
	const DirectoryEntries entries = FS.listDirectory(FS.getCampaignsDir());
	for (const std::string& e : entries) {
		if (string::endsWith(e, ".lua"))
			out.push_back(e);
	}
	std::sort(out.begin(), out.end());
	return out;
}

std::vector<std::string> MapEditorDocument::listMapsInCampaign (const std::string& campaignFile) const
{
	std::vector<std::string> out;
	collectCampaignMapIds(readCampaignSource(campaignFile), out);
	return out;
}

std::vector<std::string> MapEditorDocument::campaignsContainingMap () const
{
	std::vector<std::string> out;
	if (_fileName.empty())
		return out;
	for (const std::string& file : listCampaignFiles()) {
		for (const std::string& pattern : listMapsInCampaign(file)) {
			if (campaignPatternMatches(pattern, _fileName)) {
				out.push_back(file);
				break;
			}
		}
	}
	return out;
}

bool MapEditorDocument::addToCampaign (const std::string& campaignFile)
{
	if (_fileName.empty() || campaignFile.empty())
		return false;
	std::string source = readCampaignSource(campaignFile);
	if (source.empty())
		return false;
	for (const std::string& pattern : listMapsInCampaign(campaignFile)) {
		if (campaignPatternMatches(pattern, _fileName))
			return true;
	}
	const std::string needle = campaignAddLine(_fileName);
	const std::string line = needle + "\n";
	const size_t unlock = source.find("c:unlock()");
	if (unlock != std::string::npos)
		source.insert(unlock, line);
	else
		source += line;
	return writeCampaignSource(campaignFile, source);
}

bool MapEditorDocument::removeFromCampaign (const std::string& campaignFile)
{
	if (_fileName.empty() || campaignFile.empty())
		return false;
	std::string source = readCampaignSource(campaignFile);
	if (source.empty())
		return false;
	const std::string needle = campaignAddLine(_fileName);
	size_t pos = source.find(needle);
	if (pos == std::string::npos) {
		const std::string alt = "c:addMap(\"" + _fileName + "\")";
		pos = source.find(alt);
		if (pos == std::string::npos)
			return true;
		size_t end = pos + alt.size();
		if (end < source.size() && source[end] == '\n')
			++end;
		source.erase(pos, end - pos);
		return writeCampaignSource(campaignFile, source);
	}
	size_t end = pos + needle.size();
	if (end < source.size() && source[end] == '\n')
		++end;
	source.erase(pos, end - pos);
	return writeCampaignSource(campaignFile, source);
}

bool MapEditorDocument::createCampaign (const std::string& campaignFile, const std::string& campaignId, const std::string& text)
{
	if (campaignFile.empty() || campaignId.empty())
		return false;
	std::string name = campaignFile;
	if (!string::endsWith(name, ".lua"))
		name += ".lua";
	if (campaignId.find('"') != std::string::npos || text.find('"') != std::string::npos)
		return false;
	const FilePtr existing = FS.getFile(FS.getCampaignsDir() + name);
	if (existing && existing->exists())
		return false;
	std::string source = "-- create a new campaign\nlocal c = Campaign.new(\"";
	source += campaignId;
	source += "\")\nc:setSetting(\"icon\", \"icon-campaign\")\n";
	source += "c:setSetting(\"text\", \"";
	source += text.empty() ? campaignId : text;
	source += "\")\n";
	if (!_fileName.empty()) {
		source += campaignAddLine(_fileName);
		source += "\n";
	}
	source += "c:unlock()\n";
	return writeCampaignSource(name, source);
}

}
