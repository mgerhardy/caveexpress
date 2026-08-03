#include "MapEditorDocument.h"
#include "common/Log.h"
#include "common/String.h"
#include "common/MapSettings.h"
#include "common/KeyValueParser.h"
#include "caveexpress/shared/CaveExpressMapContext.h"
#include "caveexpress/shared/CaveExpressSpriteType.h"
#include "caveexpress/shared/CaveExpressEntityType.h"
#include "caveexpress/shared/CaveExpressAnimation.h"
#include "caveexpress/shared/constants/EmitterSettings.h"
#include "caveexpress/shared/CaveTileDefinition.h"
#include "caveexpress/server/map/RandomMapContext.h"
#include "caveexpress/server/entities/EntityEmitter.h"
#include <cmath>
#include <map>

namespace caveexpress {

MapEditorDocument::MapEditorDocument (IMapManager& mapManager) :
		IMapEditorDocument(mapManager)
{
	doClear();
}

MapEditorLayer MapEditorDocument::getLayer (const SpriteType& type) const
{
	if (SpriteTypes::isRock(type) || SpriteTypes::isAnyGround(type) || SpriteTypes::isWaterFall(type))
		return LAYER_SOLID;
	if (SpriteTypes::isBackground(type) || SpriteTypes::isWindow(type) || SpriteTypes::isCave(type))
		return LAYER_BACKGROUND;
	if (SpriteTypes::isBridge(type))
		return LAYER_FOREGROUND;
	if (SpriteTypes::isLiane(type))
		return LAYER_DECORATION;
	return LAYER_SOLID;
}

bool MapEditorDocument::isMapTileType (const SpriteType& type) const
{
	return SpriteTypes::isMapTile(type);
}

bool MapEditorDocument::requiresBackgroundTile (const SpriteType& type) const
{
	// Foreground/decoration overlays that only make sense on open background cells
	// (e.g. bridges, lianes). Solid rock/ground cells are not valid hosts.
	return SpriteTypes::isBridge(type) || SpriteTypes::isLiane(type);
}

bool MapEditorDocument::isHangingGroundSprite (const SpriteDefPtr& def) const
{
	if (!def)
		return false;
	// Thin ground decks and all ground-ledge ends need open air beneath.
	if (SpriteTypes::isGroundLeft(def->type) || SpriteTypes::isGroundRight(def->type))
		return true;
	if (!SpriteTypes::isGround(def->type))
		return false;
	return def->id.find("ground-05") != std::string::npos || def->id.find("ground-06") != std::string::npos;
}

bool MapEditorDocument::hasBackgroundCovering (gridCoord gridX, gridCoord gridY, gridSize width, gridSize height) const
{
	if (width <= 0.0f || height <= 0.0f)
		return false;

	const int x0 = static_cast<int>(std::floor(gridX + EPSILON));
	const int y0 = static_cast<int>(std::floor(gridY + EPSILON));
	const int x1 = static_cast<int>(std::ceil(gridX + width - EPSILON));
	const int y1 = static_cast<int>(std::ceil(gridY + height - EPSILON));
	if (x1 <= x0 || y1 <= y0)
		return false;

	for (int cy = y0; cy < y1; ++cy) {
		for (int cx = x0; cx < x1; ++cx) {
			bool covered = false;
			for (const MapEditorTileItem& item : _map) {
				if (!item.def || !SpriteTypes::isBackground(item.def->type))
					continue;
				if (IMapEditorDocument::isOverlapping(static_cast<gridCoord>(cx), static_cast<gridCoord>(cy), item)) {
					covered = true;
					break;
				}
			}
			if (!covered)
				return false;
		}
	}
	return true;
}

bool MapEditorDocument::hasAirBelow (gridCoord gridX, gridCoord gridY) const
{
	const int cx = static_cast<int>(std::floor(gridX + EPSILON));
	const int cy = static_cast<int>(std::floor(gridY + EPSILON)) + 1;
	if (cy >= _mapHeight)
		return true;
	for (const MapEditorTileItem& item : _map) {
		if (!item.def)
			continue;
		if (!IMapEditorDocument::isOverlapping(static_cast<gridCoord>(cx), static_cast<gridCoord>(cy), item))
			continue;
		const SpriteType& type = item.def->type;
		if (SpriteTypes::isBackground(type) || SpriteTypes::isWindow(type) || SpriteTypes::isLiane(type))
			continue;
		if (SpriteTypes::isSolid(type) || SpriteTypes::isCave(type) || SpriteTypes::isPackageTarget(type))
			return false;
	}
	return true;
}

bool MapEditorDocument::hasBridgeSideNeighbors (gridCoord gridX, gridCoord gridY) const
{
	auto isGroundOrBridgeAt = [this] (int cx, int cy) {
		if (cx < 0 || cy < 0 || cx >= _mapWidth || cy >= _mapHeight)
			return false;
		for (const MapEditorTileItem& item : _map) {
			if (!item.def)
				continue;
			if (!IMapEditorDocument::isOverlapping(static_cast<gridCoord>(cx), static_cast<gridCoord>(cy), item))
				continue;
			const SpriteType& type = item.def->type;
			if (SpriteTypes::isAnyGround(type) || SpriteTypes::isBridge(type))
				return true;
		}
		return false;
	};
	const int cx = static_cast<int>(std::floor(gridX + EPSILON));
	const int cy = static_cast<int>(std::floor(gridY + EPSILON));
	return isGroundOrBridgeAt(cx - 1, cy) || isGroundOrBridgeAt(cx + 1, cy);
}

bool MapEditorDocument::canPlaceTileItem (const MapEditorTileItem& item) const
{
	if (!item.def)
		return true;

	const vec2& size = item.getSize(true);
	const gridCoord x = item.gridX + item.getX(true);
	const gridCoord y = item.gridY + item.getY(true);

	if (requiresBackgroundTile(item.def->type)) {
		if (!hasBackgroundCovering(x, y, size.x, size.y))
			return false;
		if (SpriteTypes::isBridge(item.def->type) && !hasBridgeSideNeighbors(item.gridX, item.gridY))
			return false;
		return true;
	}

	if (isHangingGroundSprite(item.def) && !hasAirBelow(item.gridX, item.gridY))
		return false;

	return true;
}

bool MapEditorDocument::isPlayerType (const EntityType& type) const
{
	return EntityTypes::isPlayer(type);
}

const Animation& MapEditorDocument::getEmitterAnimation (const EntityType& type) const
{
	return EntityTypes::hasDirection(type) ? Animations::ANIMATION_IDLE_RIGHT : Animations::ANIMATION_IDLE;
}

const EntityType& MapEditorDocument::getPlayerEntityType () const
{
	return EntityTypes::PLAYER;
}

void MapEditorDocument::setWaterParameters (float waterHeight, float waterChangeSpeed, uint32_t waterRisingDelay,
		uint32_t waterFallingDelay)
{
	setWaterHeight(waterHeight);
	setSetting(msn::WATER_CHANGE, string::toString(waterChangeSpeed));
	setSetting(msn::WATER_RISING_DELAY, string::toString(waterRisingDelay));
	setSetting(msn::WATER_FALLING_DELAY, string::toString(waterFallingDelay));
}

void MapEditorDocument::setWaterHeight (float waterHeight)
{
	_waterHeight = waterHeight;
	if (_waterHeight > _mapHeight)
		_waterHeight = string::toFloat(msd::WATER_HEIGHT);
	_settings[msn::WATER_HEIGHT] = string::toString(waterHeight);
}

void MapEditorDocument::doClear ()
{
	IMapEditorDocument::doClear();
	setSetting(msn::PACKAGE_TRANSFER_COUNT, msd::PACKAGE_TRANSFER_COUNT);
	setSetting(msn::FLYING_NPC, msd::FLYING_NPC);
	setSetting(msn::FISH_NPC, msd::FISH_NPC);
	setSetting(msn::SIDEBORDERFAIL, msd::SIDEBORDERFAIL);
	setSetting(msn::WIND, msd::WIND);
	setSetting(msn::GRAVITY, string::toString(msdv::GRAVITY));
	setWaterParameters(string::toFloat(msd::WATER_HEIGHT), string::toFloat(msd::WATER_CHANGE),
			string::toInt(msd::WATER_RISING_DELAY), string::toInt(msd::WATER_FALLING_DELAY));
}

void MapEditorDocument::onAfterStateRestored ()
{
	setWaterHeight(string::toFloat(_settings[msn::WATER_HEIGHT]));
}

void MapEditorDocument::setActiveEntityRight (bool right)
{
	if (_activeEntityType == nullptr || !EntityTypes::hasDirection(*_activeEntityType))
		return;
	_activeEntityRight = right;
	const Animation& animation = right ? Animations::ANIMATION_IDLE_RIGHT : Animations::ANIMATION_IDLE_LEFT;
	const SpriteDefPtr& spriteDef = SpriteDefinition::get().getFromEntityType(*_activeEntityType, animation);
	if (spriteDef)
		_activeSprite = spriteDef;
}

void MapEditorDocument::rotateBrush ()
{
	if (_activeEntityType != nullptr && EntityTypes::hasDirection(*_activeEntityType)) {
		setActiveEntityRight(!_activeEntityRight);
		return;
	}
	IMapEditorDocument::rotateBrush();
}

bool MapEditorDocument::placeCave (const SpriteDefPtr& def, const EntityType* entityType, gridCoord gridX, gridCoord gridY,
		MapEditorLayer layer, int delay, bool overwrite)
{
	MapEditorTileItem item;
	item.def = def;
	item.entityType = entityType;
	item.delay = delay;
	item.gridX = gridX;
	item.gridY = gridY;
	item.layer = layer;
	item.mapTile = true;
	return placeTileItem(item, overwrite);
}

bool MapEditorDocument::placeBrushItem (bool overwrite)
{
	if (!_activeSprite)
		return false;
	if (_activeEntityType != nullptr && isPlayerType(*_activeEntityType)) {
		setPlayerPosition(_selectedGridX, _selectedGridY);
		return true;
	}
	if (SpriteTypes::isCave(_activeSprite->type))
		return placeCave(_activeSprite, &EntityType::NONE, _selectedGridX, _selectedGridY, _activeLayer, _caveDelay, overwrite);

	if (_activeEntityType != nullptr) {
		std::string settings;
		if (EntityTypes::hasDirection(*_activeEntityType) && !_activeEntityRight)
			settings = EMITTER_RIGHT "=false";
		MapEditorTileItem item;
		item.def = _activeSprite;
		item.entityType = _activeEntityType;
		item.amount = _emitterAmount;
		item.delay = _emitterDelay;
		item.gridX = _selectedGridX;
		item.gridY = _selectedGridY;
		item.layer = LAYER_EMITTER;
		item.angle = _activeAngle;
		item.settings = settings;
		item.mapTile = false;
		if (!canPlaceTileItem(item))
			return false;
		return placeTileItem(item, overwrite);
	}

	MapEditorTileItem item;
	item.def = _activeSprite;
	item.gridX = _selectedGridX;
	item.gridY = _selectedGridY;
	item.layer = _activeLayer;
	item.angle = _activeAngle;
	item.mapTile = isMapTileType(_activeSprite->type);
	if (!canPlaceTileItem(item))
		return false;
	return placeTileItem(item, overwrite);
}

bool MapEditorDocument::isOverlapping (const MapEditorTileItem& item1, const MapEditorTileItem& item2) const
{
	switch (item1.layer) {
	case LAYER_BACKGROUND:
		if (item2.layer == LAYER_FOREGROUND || item2.layer == LAYER_DECORATION || item2.layer == LAYER_EMITTER)
			return false;
		break;
	case LAYER_FOREGROUND:
		if (item2.layer == LAYER_BACKGROUND || item2.layer == LAYER_DECORATION || item2.layer == LAYER_EMITTER)
			return false;
		break;
	case LAYER_DECORATION:
	case LAYER_EMITTER:
		if (item2.entityType != nullptr && EntityTypes::isNpc(*item2.entityType))
			break;
		if (item2.layer != LAYER_SOLID)
			return false;
		break;
	default:
		break;
	}
	const vec2& size = item1.getSize(true);
	const gridCoord x = item1.gridX + item1.getX(true) + EPSILON;
	const gridCoord y = item1.gridY + item1.getY(true) + EPSILON;
	return IMapEditorDocument::isOverlapping(x, y, size.x - 2.0f * EPSILON, size.y - 2.0f * EPSILON, item2);
}

bool MapEditorDocument::shouldSaveTile (const MapEditorTileItem& tile) const
{
	return tile.entityType == nullptr && !SpriteTypes::isCave(tile.def->type);
}

bool MapEditorDocument::shouldSaveEmitter (const MapEditorTileItem& tile) const
{
	return tile.entityType != nullptr && !SpriteTypes::isCave(tile.def->type);
}

void MapEditorDocument::prepareContextForSaving (IMapContext& ctx)
{
	IMapEditorDocument::prepareContextForSaving(ctx);
	MapEditorTileItems map = _map;
	map.sort();
	std::vector<CaveTileDefinition> caves;
	for (const MapEditorTileItem& item : map) {
		if (item.gridX >= _mapWidth || item.gridY >= _mapHeight)
			continue;
		if (!SpriteTypes::isCave(item.def->type))
			continue;
		caves.emplace_back(item.gridX, item.gridY, item.def, *item.entityType, item.delay);
	}
	static_cast<CaveExpressMapContext&>(ctx).setCaveTileDefinitions(caves);
}

void MapEditorDocument::loadFromContext (IMapContext& ctx)
{
	ctx.load(true);
	setFileName(ctx.getName());
	setMapName(ctx.getTitle());
	_lastMap->setValue(ctx.getName());

	for (const auto& setting : ctx.getSettings()) {
		if (setting.first == msn::WATER_HEIGHT)
			setWaterHeight(string::toFloat(setting.second));
		else if (setting.first == msn::THEME)
			setTheme(ThemeType::getByName(setting.second));
		else
			setSetting(setting.first, setting.second);
	}
	_startPositions = ctx.getStartPositions();
	setMapDimensions(string::toInt(_settings[msn::WIDTH]), string::toInt(_settings[msn::HEIGHT]));

	for (const MapTileDefinition& tile : ctx.getMapTileDefinitions()) {
		MapEditorTileItem item;
		item.def = tile.spriteDef;
		item.gridX = tile.x;
		item.gridY = tile.y;
		item.layer = getLayer(tile.spriteDef->type);
		item.angle = tile.angle;
		item.mapTile = true;
		if (!placeTileItem(item, false))
			Log::error(LOG_GAMEIMPL, "could not place tile %s", tile.spriteDef->id.c_str());
	}
	for (const CaveTileDefinition& cave : static_cast<CaveExpressMapContext&>(ctx).getCaveTileDefinitions()) {
		if (!placeCave(cave.spriteDef, cave.type, cave.x, cave.y, getLayer(cave.spriteDef->type), cave.delay, false))
			Log::error(LOG_GAMEIMPL, "could not place cave %s", cave.spriteDef->id.c_str());
	}
	for (const EmitterDefinition& emitter : ctx.getEmitterDefinitions()) {
		const EntityType& entityType = *emitter.type;
		const KeyValueParser s(emitter.settings);
		const Animation& animation = EntityTypes::hasDirection(entityType) ?
				(s.getBool(EMITTER_RIGHT, true) ? Animations::ANIMATION_IDLE_RIGHT : Animations::ANIMATION_IDLE_LEFT) :
				Animations::ANIMATION_IDLE;
		const SpriteDefPtr def = SpriteDefinition::get().getFromEntityType(entityType, animation);
		if (!def)
			continue;
		MapEditorTileItem item;
		item.def = def;
		item.entityType = &entityType;
		item.amount = emitter.amount;
		item.delay = emitter.delay;
		item.gridX = emitter.x;
		item.gridY = emitter.y;
		item.layer = LAYER_EMITTER;
		item.angle = s.getFloat(EMITTER_ANGLE);
		item.settings = emitter.settings;
		item.mapTile = false;
		if (!placeTileItem(item, false))
			Log::error(LOG_GAMEIMPL, "could not place emitter %s", entityType.name.c_str());
	}
}

std::unique_ptr<IMapContext> MapEditorDocument::createContext (const std::string& mapName) const
{
	return std::unique_ptr<IMapContext>(new CaveExpressMapContext(mapName));
}

void MapEditorDocument::fillTilePalette (std::vector<SpriteDefPtr>& out) const
{
	for (SpriteDefMapConstIter i = SpriteDefinition::get().begin(); i != SpriteDefinition::get().end(); ++i) {
		const SpriteDefPtr& sprite = i->second;
		if (!sprite->theme.isNone() && sprite->theme != getTheme())
			continue;
		const SpriteType& type = sprite->type;
		if (!SpriteTypes::isMapTile(type) && !SpriteTypes::isLiane(type))
			continue;
		if (SpriteTypes::isGeyser(type) && sprite->isStatic())
			continue;
		if (SpriteTypes::isPackageTarget(type) && !sprite->isStatic())
			continue;
		out.push_back(sprite);
	}
}

void MapEditorDocument::fillEntityPalette (std::vector<const EntityType*>& out) const
{
	out.push_back(&EntityTypes::PLAYER);
	const EntityType** types = EntityEmitter::getSupportedEntityTypes();
	for (; types && *types; ++types)
		out.push_back(*types);
}

void MapEditorDocument::autoFill (const ThemeType& theme)
{
	MapEditorUndo();
	const std::string name = "editor-random-" + theme.name;
	RandomMapContext ctx(name, theme, _mapWidth, _mapHeight);
	ctx.setSettings(_settings);
	const unsigned int seed = static_cast<unsigned int>(string::toInt(getSetting("seed", "0")));
	if (seed != 0)
		ctx.setSeed(seed);
	const std::string oldName = _fileName;
	loadFromContext(ctx);
	setFileName(oldName);
}

void MapEditorDocument::changeMapTheme (const ThemeType& toTheme)
{
	std::map<std::string, std::string> replaces;
	if (toTheme == ThemeTypes::JUNGLE) {
		replaces = {
			{"tile-background-01", "tile-background-jungle-01"},
			{"tile-background-02", "tile-background-jungle-02"},
			{"tile-background-03", "tile-background-jungle-03"},
			{"tile-background-04", "tile-background-jungle-04"},
			{"tile-cave-01", "tile-cave-jungle-01"},
			{"tile-cave-02", "tile-cave-jungle-02"},
			{"tile-ground-01", "tile-ground-jungle-01"},
			{"tile-ground-02", "tile-ground-jungle-02"},
			{"tile-rock-01", "tile-rock-jungle-01"},
			{"tile-rock-02", "tile-rock-jungle-02"},
			{"tile-rock-03", "tile-rock-jungle-03"},
		};
	} else if (toTheme == ThemeTypes::DESERT) {
		replaces = {
			{"tile-background-ice-01", "tile-background-desert-01"},
			{"tile-cave-ice-01", "tile-cave-desert-01"},
			{"tile-ground-ice-01", "tile-ground-desert-01"},
			{"tile-rock-ice-01", "tile-rock-desert-01"},
		};
	} else {
		setTheme(toTheme);
		return;
	}
	MapEditorUndo();
	int replaced = 0;
	for (MapEditorTileItem& tile : _map) {
		const auto i = replaces.find(tile.def->id);
		if (i == replaces.end())
			continue;
		const SpriteDefPtr next = SpriteDefinition::get().getSpriteDefinition(i->second);
		if (!next)
			continue;
		tile.def = next;
		++replaced;
	}
	setTheme(toTheme);
}

}
