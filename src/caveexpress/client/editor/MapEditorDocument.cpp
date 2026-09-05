#include "MapEditorDocument.h"
#include "common/Log.h"
#include "common/String.h"
#include "common/Math.h"
#include "common/MapSettings.h"
#include "common/ThemeType.h"
#include "common/KeyValueParser.h"
#include "common/vec2.h"
#include "caveexpress/shared/CaveExpressMapContext.h"
#include "caveexpress/shared/CaveExpressSpriteType.h"
#include "caveexpress/shared/CaveExpressEntityType.h"
#include "caveexpress/shared/CaveExpressAnimation.h"
#include "caveexpress/shared/constants/EmitterSettings.h"
#include "caveexpress/shared/CaveTileDefinition.h"
#include "caveexpress/shared/GateDefinition.h"
#include "caveexpress/shared/PressurePlateDefinition.h"
#include "caveexpress/shared/MapValidator.h"
#include "caveexpress/server/map/RandomMapContext.h"
#include "caveexpress/server/entities/EntityEmitter.h"
#include "common/FileSystem.h"
#include "common/File.h"
#include <algorithm>
#include <cmath>
#include <cstring>
#include <map>
#include <set>

namespace caveexpress {

MapEditorDocument::MapEditorDocument (IMapManager& mapManager) :
		IMapEditorDocument(mapManager)
{
	doClear();
}

MapEditorLayer MapEditorDocument::getLayer (const SpriteType& type) const
{
	if (SpriteTypes::isRock(type) || SpriteTypes::isAnyGround(type) || SpriteTypes::isWaterFall(type)
			|| SpriteTypes::isGate(type) || SpriteTypes::isPressurePlate(type))
		return LAYER_SOLID;
	if (SpriteTypes::isBackground(type) || SpriteTypes::isWindow(type) || SpriteTypes::isCave(type))
		return LAYER_BACKGROUND;
	if (SpriteTypes::isBridge(type))
		return LAYER_FOREGROUND;
	if (SpriteTypes::isLiane(type) || SpriteTypes::isCaveSign(type) || type.isNone())
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

void MapEditorDocument::rotateSelectionOrBrush ()
{
	if (_highlightItem != nullptr && _highlightItem->def && _highlightItem->entityType != nullptr
			&& EntityTypes::hasDirection(*_highlightItem->entityType)
			&& !SpriteTypes::isCave(_highlightItem->def->type)) {
		KeyValueParser kv(_highlightItem->settings);
		const bool right = !kv.getBool(EMITTER_RIGHT, true);
		if (right)
			kv.remove(EMITTER_RIGHT);
		else
			kv.set(EMITTER_RIGHT, false);
		_highlightItem->settings = kv.str();
		setActiveEntityRight(right);
		const Animation& animation = right ? Animations::ANIMATION_IDLE_RIGHT : Animations::ANIMATION_IDLE_LEFT;
		const SpriteDefPtr def = SpriteDefinition::get().getFromEntityType(*_highlightItem->entityType, animation);
		if (def)
			_highlightItem->def = def;
		return;
	}
	IMapEditorDocument::rotateSelectionOrBrush();
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
	if (_pickGateTarget) {
		MapEditorTileItem* gate = findTileAt(_selectedGridX, _selectedGridY);
		MapEditorTileItem* plate = findTileAt(_linkPlateX, _linkPlateY);
		if (gate != nullptr && SpriteTypes::isGate(gate->def->type) && plate != nullptr
				&& SpriteTypes::isPressurePlate(plate->def->type)) {
			MapEditorUndo();
			if (plate->linkId.empty())
				plate->linkId = string::format("link-%d-%d", (int)plate->gridX, (int)plate->gridY);
			gate->linkId = plate->linkId;
			_pickGateTarget = false;
			return true;
		}
		_pickGateTarget = false;
		return false;
	}

	if (!_activeSprite)
		return false;
	if (_activeEntityType != nullptr && isPlayerType(*_activeEntityType)) {
		setPlayerPosition(_selectedGridX, _selectedGridY);
		return true;
	}
	if (SpriteTypes::isCave(_activeSprite->type))
		return placeCave(_activeSprite, _caveNpcType, _selectedGridX, _selectedGridY, _activeLayer, _caveDelay, overwrite);

	if (_activeEntityType != nullptr) {
		KeyValueParser settings("");
		if (EntityTypes::hasDirection(*_activeEntityType) && !_activeEntityRight)
			settings.set(EMITTER_RIGHT, false);
		if (EntityTypes::isNpcBlowing(*_activeEntityType)) {
			settings.set(EMITTER_STRENGTH, 10.0f);
			settings.set(EMITTER_WIND_MOD_SIZE, 2.0f);
		}
		MapEditorTileItem item;
		item.def = _activeSprite;
		item.entityType = _activeEntityType;
		item.amount = _emitterAmount;
		item.delay = _emitterDelay;
		item.gridX = _selectedGridX;
		item.gridY = _selectedGridY;
		item.layer = LAYER_EMITTER;
		item.angle = _activeAngle;
		item.settings = settings.str();
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
		// Background may share a cell with solid overlays (gates, plates, ground) and other layers.
		if (item2.layer == LAYER_FOREGROUND || item2.layer == LAYER_DECORATION || item2.layer == LAYER_EMITTER
				|| item2.layer == LAYER_SOLID)
			return false;
		break;
	case LAYER_FOREGROUND:
		if (item2.layer == LAYER_BACKGROUND || item2.layer == LAYER_DECORATION || item2.layer == LAYER_EMITTER
				|| item2.layer == LAYER_SOLID)
			return false;
		break;
	case LAYER_SOLID:
		// Gates / plates / caves sit on background cells in corridor maps.
		if (item2.layer == LAYER_BACKGROUND || item2.layer == LAYER_FOREGROUND || item2.layer == LAYER_DECORATION
				|| item2.layer == LAYER_EMITTER)
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
	return tile.entityType == nullptr && !SpriteTypes::isCave(tile.def->type)
			&& !SpriteTypes::isGate(tile.def->type) && !SpriteTypes::isPressurePlate(tile.def->type);
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
	std::vector<GateDefinition> gates;
	std::vector<PressurePlateDefinition> plates;
	for (const MapEditorTileItem& item : map) {
		if (item.gridX >= _mapWidth || item.gridY >= _mapHeight)
			continue;
		if (SpriteTypes::isCave(item.def->type)) {
			const EntityType& npc = item.entityType != nullptr ? *item.entityType : EntityType::NONE;
			caves.emplace_back(item.gridX, item.gridY, item.def, npc, item.delay);
		} else if (SpriteTypes::isGate(item.def->type)) {
			gates.emplace_back(item.gridX, item.gridY, item.def, item.linkId, item.openAmount);
		} else if (SpriteTypes::isPressurePlate(item.def->type)) {
			plates.emplace_back(item.gridX, item.gridY, item.def, item.linkId, item.requiredWeight, item.delay);
		}
	}
	CaveExpressMapContext& ceCtx = static_cast<CaveExpressMapContext&>(ctx);
	ceCtx.setCaveTileDefinitions(caves);
	ceCtx.setGateDefinitions(gates);
	ceCtx.setPressurePlateDefinitions(plates);
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
	CaveExpressMapContext& ceCtx = static_cast<CaveExpressMapContext&>(ctx);
	for (const GateDefinition& gate : ceCtx.getGateDefinitions()) {
		MapEditorTileItem item;
		item.def = gate.spriteDef;
		item.gridX = gate.x;
		item.gridY = gate.y;
		item.layer = getLayer(gate.spriteDef->type);
		item.linkId = gate.linkId;
		item.openAmount = gate.openAmount;
		item.mapTile = true;
		if (!placeTileItem(item, false))
			Log::error(LOG_GAMEIMPL, "could not place gate %s", gate.spriteDef->id.c_str());
	}
	for (const PressurePlateDefinition& plate : ceCtx.getPressurePlateDefinitions()) {
		MapEditorTileItem item;
		item.def = plate.spriteDef;
		item.gridX = plate.x;
		item.gridY = plate.y;
		item.layer = getLayer(plate.spriteDef->type);
		item.linkId = plate.linkId;
		item.requiredWeight = plate.requiredWeight;
		item.delay = plate.holdMs;
		item.mapTile = true;
		if (!placeTileItem(item, false))
			Log::error(LOG_GAMEIMPL, "could not place pressure plate %s", plate.spriteDef->id.c_str());
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
	_scriptLogic = ctx.getScriptLogic();
	_scriptDirty = false;
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
		if (!SpriteTypes::isMapTile(type) && !SpriteTypes::isLiane(type) && !SpriteTypes::isCaveSign(type)
				&& sprite->id != "dust" && sprite->id != "waste")
			continue;
		if (sprite->hasNoTextures())
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

void MapEditorDocument::beginPickGateTarget ()
{
	MapEditorTileItem* plate = getHighlightItem();
	if (plate == nullptr || plate->def == nullptr || !SpriteTypes::isPressurePlate(plate->def->type))
		return;
	_linkPlateX = plate->gridX;
	_linkPlateY = plate->gridY;
	_pickGateTarget = true;
}

MapEditorTileItem* MapEditorDocument::findTileAt (gridCoord gridX, gridCoord gridY)
{
	for (MapEditorTileItem& item : _map) {
		if (fequals(item.gridX, gridX) && fequals(item.gridY, gridY))
			return &item;
	}
	return nullptr;
}

 const MapEditorTileItem* MapEditorDocument::findLinkedPartner (const MapEditorTileItem& item) const
{
	if (item.linkId.empty() || item.def == nullptr)
		return nullptr;
	const bool wantGate = SpriteTypes::isPressurePlate(item.def->type);
	const bool wantPlate = SpriteTypes::isGate(item.def->type);
	if (!wantGate && !wantPlate)
		return nullptr;
	for (const MapEditorTileItem& other : _map) {
		if (&other == &item || other.linkId != item.linkId || other.def == nullptr)
			continue;
		if (wantGate && SpriteTypes::isGate(other.def->type))
			return &other;
		if (wantPlate && SpriteTypes::isPressurePlate(other.def->type))
			return &other;
	}
	return nullptr;
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

namespace {

std::map<std::string, std::string> themeReplaceMap (const ThemeType& toTheme)
{
	if (toTheme == ThemeTypes::JUNGLE) {
		return {
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
	}
	if (toTheme == ThemeTypes::DESERT) {
		return {
			{"tile-background-ice-01", "tile-background-desert-01"},
			{"tile-cave-ice-01", "tile-cave-desert-01"},
			{"tile-ground-ice-01", "tile-ground-desert-01"},
			{"tile-rock-ice-01", "tile-rock-desert-01"},
		};
	}
	return {};
}

bool idLooksThemed (const std::string& id)
{
	return id.find("jungle") != std::string::npos || id.find("desert") != std::string::npos
			|| id.find("-ice-") != std::string::npos || string::endsWith(id, "-ice");
}

}

void MapEditorDocument::previewThemeRemap (const ThemeType& toTheme, int& wouldReplace, int& leftoverThemed) const
{
	wouldReplace = 0;
	leftoverThemed = 0;
	const auto replaces = themeReplaceMap(toTheme);
	for (const MapEditorTileItem& tile : _map) {
		if (!tile.def)
			continue;
		if (replaces.find(tile.def->id) != replaces.end())
			++wouldReplace;
		else if (idLooksThemed(tile.def->id) && tile.def->theme != toTheme && !tile.def->theme.isNone())
			++leftoverThemed;
	}
}

void MapEditorDocument::changeMapTheme (const ThemeType& toTheme)
{
	const auto replaces = themeReplaceMap(toTheme);
	if (replaces.empty()) {
		setTheme(toTheme);
		return;
	}
	MapEditorUndo();
	for (MapEditorTileItem& tile : _map) {
		if (!tile.def)
			continue;
		const auto i = replaces.find(tile.def->id);
		if (i == replaces.end())
			continue;
		const SpriteDefPtr next = SpriteDefinition::get().getSpriteDefinition(i->second);
		if (!next)
			continue;
		tile.def = next;
	}
	setTheme(toTheme);
}

MapMetrics MapEditorDocument::evaluateLayout () const
{
	std::vector<MapTileDefinition> tiles;
	std::vector<CaveTileDefinition> caves;
	std::vector<EmitterDefinition> emitters;
	for (const MapEditorTileItem& item : _map) {
		if (!item.def)
			continue;
		if (item.gridX >= _mapWidth || item.gridY >= _mapHeight)
			continue;
		if (SpriteTypes::isCave(item.def->type)) {
			const EntityType& npc = item.entityType != nullptr ? *item.entityType : EntityType::NONE;
			caves.emplace_back(item.gridX, item.gridY, item.def, npc, item.delay);
		} else if (item.entityType != nullptr) {
			emitters.emplace_back(item.gridX, item.gridY, *item.entityType, item.amount, item.delay, item.settings);
		} else {
			tiles.emplace_back(item.gridX, item.gridY, item.def, item.angle);
		}
	}
	return MapValidator().evaluate(_mapWidth, _mapHeight, tiles, caves, emitters, _startPositions);
}

void MapEditorDocument::collectGameValidationIssues (std::vector<std::string>& out) const
{
	const bool cutscene = string::toBool(getSetting(msn::CUTSCENE, msd::CUTSCENE));
	if (!cutscene) {
		for (const IMap::StartPosition& pos : _startPositions) {
			const gridCoord x = string::toFloat(pos._x);
			const gridCoord y = string::toFloat(pos._y);
			for (const MapEditorTileItem& item : _map) {
				if (!item.def || !SpriteTypes::isSolid(item.def->type))
					continue;
				if (IMapEditorDocument::isOverlapping(x, y, item)) {
					out.push_back("Start position is blocked by solid tiles");
					break;
				}
			}
		}
	}
	const int packages = string::toInt(getSetting(msn::PACKAGE_TRANSFER_COUNT, msd::PACKAGE_TRANSFER_COUNT));
	const int npcs = string::toInt(getSetting(msn::NPC_TRANSFER_COUNT, msd::NPC_TRANSFER_COUNT));
	bool hasShredder = false;
	bool hasCave = false;
	bool hasPackage = false;
	bool hasRockPackage = false;
	bool hasIcePackage = false;
	bool hasGeyser = false;
	for (const MapEditorTileItem& item : _map) {
		if (!item.def)
			continue;
		if (SpriteTypes::isPackageTarget(item.def->type))
			hasShredder = true;
		if (SpriteTypes::isCave(item.def->type))
			hasCave = true;
		if (SpriteTypes::isGeyser(item.def->type))
			hasGeyser = true;
		if (item.entityType != nullptr && EntityTypes::isPackage(*item.entityType)) {
			hasPackage = true;
			if (EntityTypes::isPackage(*item.entityType) && item.entityType == &EntityTypes::PACKAGE_ICE)
				hasIcePackage = true;
			else
				hasRockPackage = true;
		}
	}
	if (packages > 0 && !hasShredder)
		out.push_back("packagetransfercount > 0 but no shredder / package target");
	if (packages > 0 && !hasPackage && !hasCave)
		out.push_back("packagetransfercount > 0 but no package emitter or cave");
	int caveCount = 0;
	for (const MapEditorTileItem& item : _map) {
		if (item.def && SpriteTypes::isCave(item.def->type))
			++caveCount;
	}
	if (npcs > 0 && caveCount < 2)
		out.push_back("npctransfercount > 0 needs at least two caves (pickup and destination)");

	const bool fish = string::toBool(getSetting(msn::FISH_NPC, msd::FISH_NPC));
	if (fish && _waterHeight <= 0.0f)
		out.push_back("fishnpc is on but waterheight is 0");
	const float waterChange = string::toFloat(getSetting(msn::WATER_CHANGE, msd::WATER_CHANGE));
	if (std::fabs(waterChange) > 0.0001f && _waterHeight <= 0.0f)
		out.push_back("water is moving but waterheight is 0");
	if (hasGeyser && string::toInt(getSetting(msn::GEYSER_INITIAL_DELAY_TIME, "3000")) < 0)
		out.push_back("geyserinitialdelay is negative");
	if (getTheme() == ThemeTypes::ICE && hasRockPackage && !hasIcePackage)
		out.push_back("Ice map uses rock packages (item-package-ice slides on ice)");
	if (getTheme() != ThemeTypes::ICE && hasIcePackage && !hasRockPackage)
		out.push_back("Non-ice map uses ice packages");

	if (!cutscene) {
		const MapMetrics metrics = evaluateLayout();
		if (!metrics.valid && !metrics.failureReason.empty())
			out.push_back(std::string("Layout: ") + metrics.failureReason);
	}
}

void MapEditorDocument::makePlayable ()
{
	MapEditorUndo();
	bool hasCave = false;
	bool hasShredder = false;
	bool hasPackage = false;
	for (const MapEditorTileItem& item : _map) {
		if (!item.def)
			continue;
		if (SpriteTypes::isCave(item.def->type))
			hasCave = true;
		if (SpriteTypes::isPackageTarget(item.def->type))
			hasShredder = true;
		if (item.entityType != nullptr && EntityTypes::isPackage(*item.entityType))
			hasPackage = true;
	}

	auto findSprite = [this] (bool (*pred) (const SpriteType&)) -> SpriteDefPtr {
		for (SpriteDefMapConstIter i = SpriteDefinition::get().begin(); i != SpriteDefinition::get().end(); ++i) {
			const SpriteDefPtr& sprite = i->second;
			if (!pred(sprite->type))
				continue;
			if (!sprite->theme.isNone() && sprite->theme != getTheme())
				continue;
			if (SpriteTypes::isPackageTarget(sprite->type) && !sprite->isStatic())
				continue;
			return sprite;
		}
		return SpriteDefPtr();
	};

	auto findOpenCell = [this] (int& ox, int& oy) -> bool {
		for (int y = 1; y < _mapHeight - 1; ++y) {
			for (int x = 1; x < _mapWidth - 1; ++x) {
				bool blocked = false;
				bool background = false;
				for (const MapEditorTileItem& item : _map) {
					if (!item.def)
						continue;
					if (!IMapEditorDocument::isOverlapping(static_cast<gridCoord>(x), static_cast<gridCoord>(y), item))
						continue;
					if (SpriteTypes::isBackground(item.def->type))
						background = true;
					if (SpriteTypes::isSolid(item.def->type) || SpriteTypes::isCave(item.def->type)
							|| SpriteTypes::isPackageTarget(item.def->type))
						blocked = true;
				}
				if (background && !blocked) {
					ox = x;
					oy = y;
					return true;
				}
			}
		}
		return false;
	};

	int x = 2;
	int y = 2;
	if (!hasCave) {
		const SpriteDefPtr cave = findSprite(SpriteTypes::isCave);
		if (cave && findOpenCell(x, y))
			placeCave(cave, _caveNpcType, static_cast<gridCoord>(x), static_cast<gridCoord>(y),
					getLayer(cave->type), _caveDelay, false);
	}
	if (!hasShredder) {
		const SpriteDefPtr shredder = findSprite(SpriteTypes::isPackageTarget);
		if (shredder && findOpenCell(x, y)) {
			MapEditorTileItem item;
			item.def = shredder;
			item.gridX = static_cast<gridCoord>(x);
			item.gridY = static_cast<gridCoord>(y);
			item.layer = getLayer(shredder->type);
			item.mapTile = true;
			placeTileItem(item, false);
		}
	}
	if (!hasPackage) {
		const EntityType& pkg = getTheme() == ThemeTypes::ICE ? EntityTypes::PACKAGE_ICE : EntityTypes::PACKAGE_ROCK;
		const SpriteDefPtr def = SpriteDefinition::get().getFromEntityType(pkg, Animations::ANIMATION_IDLE);
		if (def && findOpenCell(x, y)) {
			MapEditorTileItem item;
			item.def = def;
			item.entityType = &pkg;
			item.amount = 1;
			item.gridX = static_cast<gridCoord>(x);
			item.gridY = static_cast<gridCoord>(y);
			item.layer = LAYER_EMITTER;
			item.mapTile = false;
			placeTileItem(item, false);
		}
	}
	if (_startPositions.empty() && findOpenCell(x, y))
		setPlayerPosition(static_cast<gridCoord>(x), static_cast<gridCoord>(y));
	if (string::toInt(getSetting(msn::PACKAGE_TRANSFER_COUNT, msd::PACKAGE_TRANSFER_COUNT)) <= 0)
		setSetting(msn::PACKAGE_TRANSFER_COUNT, "1");
}

namespace {

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
		const std::vector<std::string> maps = listMapsInCampaign(file);
		if (std::find(maps.begin(), maps.end(), _fileName) != maps.end())
			out.push_back(file);
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
	const std::string needle = campaignAddLine(_fileName);
	if (source.find(needle) != std::string::npos)
		return true;
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
	source += "\")\nc:setSetting(\"icon\", \"icon-campaign-rock\")\n";
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

std::vector<UnpairedTrigger> MapEditorDocument::listUnpairedTriggers () const
{
	std::vector<UnpairedTrigger> out;
	for (const MapEditorTileItem& item : _map) {
		if (!item.def)
			continue;
		if (!SpriteTypes::isGate(item.def->type) && !SpriteTypes::isPressurePlate(item.def->type))
			continue;
		if (item.linkId.empty() || findLinkedPartner(item) == nullptr) {
			UnpairedTrigger t;
			t.kind = SpriteTypes::isGate(item.def->type) ? "gate" : "plate";
			t.linkId = item.linkId;
			t.x = item.gridX;
			t.y = item.gridY;
			out.push_back(t);
		}
	}
	return out;
}

}
