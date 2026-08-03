#include "ui/editor/IMapEditorDocument.h"
#include "common/Log.h"
#include "common/String.h"
#include "common/ConfigManager.h"
#include "common/CommandSystem.h"
#include "common/Commands.h"
#include "common/MapSettings.h"
#include "common/KeyValueParser.h"
#include "ui/UI.h"
#include "ui/windows/UIWindow.h"
#include <algorithm>
#include <memory>

const int IMapEditorDocument::MIN_WIDTH = 6;
const int IMapEditorDocument::MIN_HEIGHT = 4;

MapEditorStateChecker::MapEditorStateChecker (IMapEditorDocument* doc) :
		_doc(doc)
{
	// Finish any open mouse-stroke so this discrete edit gets its own undo step.
	_doc->endUndoStroke();
	_map = doc->_map;
	_settings = doc->_settings;
	_startPositions = doc->_startPositions;
	_mapName = doc->_mapName;
	_mapWidth = doc->_mapWidth;
	_mapHeight = doc->_mapHeight;
}

MapEditorStateChecker::~MapEditorStateChecker ()
{
	const IMapEditorDocument::State before(_map, _settings, _startPositions, _mapName, _mapWidth, _mapHeight);
	if (!_doc->stateDiffersFrom(before))
		return;
	_doc->_undoStates.push_back(before);
	_doc->_redoStates.clear();
}

IMapEditorDocument::IMapEditorDocument (IMapManager& mapManager) :
		_mapManager(mapManager), _theme(&ThemeTypes::ROCK)
{
	_lastMap = Config.getConfigVar("editor-lastmap");
	doClear();
}

IMapEditorDocument::~IMapEditorDocument ()
{
}

IMapEditorDocument::State IMapEditorDocument::captureState () const
{
	return State(_map, _settings, _startPositions, _mapName, _mapWidth, _mapHeight);
}

bool IMapEditorDocument::stateDiffersFrom (const State& state) const
{
	if (state.mapWidth != _mapWidth || state.mapHeight != _mapHeight || state.mapName != _mapName)
		return true;
	if (state.map.size() != _map.size() || state.settingsMap.size() != _settings.size()
			|| state.startPositions.size() != _startPositions.size())
		return true;
	if (!std::equal(state.settingsMap.begin(), state.settingsMap.end(), _settings.begin()))
		return true;
	if (!std::equal(state.startPositions.begin(), state.startPositions.end(), _startPositions.begin(),
			[] (const IMap::StartPosition& a, const IMap::StartPosition& b) {
				return a._x == b._x && a._y == b._y;
			}))
		return true;
	return !std::equal(state.map.begin(), state.map.end(), _map.begin());
}

void IMapEditorDocument::beginUndoStroke ()
{
	if (_undoStrokeActive)
		return;
	_undoStrokeSnapshot = captureState();
	_undoStrokeActive = true;
}

void IMapEditorDocument::endUndoStroke ()
{
	if (!_undoStrokeActive)
		return;
	_undoStrokeActive = false;
	if (!stateDiffersFrom(_undoStrokeSnapshot))
		return;
	_undoStates.push_back(std::move(_undoStrokeSnapshot));
	_redoStates.clear();
}

void IMapEditorDocument::registerCommands ()
{
	CommandPtr cmd = Commands.registerCommand(CMD_LOADMAP, [this] (const ICommand::Args& args) {
		if (args.size() != 1 || args[0].empty()) {
			Log::error(LOG_UI, "no map given");
			return;
		}
		UI::get().pushRoot(UI_WINDOW_EDITOR);
		load(args[0]);
	});
	cmd->setCompleter([this] (const std::string& input, std::vector<std::string>& matches) {
		for (auto entry : _mapManager.getMapsByWildcard(input + "*"))
			matches.push_back(entry.first);
	});
}

void IMapEditorDocument::unregisterCommands ()
{
	Commands.removeCommand(CMD_LOADMAP);
}

void IMapEditorDocument::doClear ()
{
	_highlightItem = nullptr;
	_map.clear();
	_settings.clear();
	_startPositions.clear();
	_lastSave = 0;
	_fileName = "newmap";
	_mapName = "A new map";
	setMapDimensions(16, 12);
	setSetting(msn::POINTS, string::toString(msdv::POINTS));
	setSetting(msn::REFERENCETIME, string::toString(msdv::REFERENCETIME));
	setTheme(ThemeTypes::ROCK);
}

void IMapEditorDocument::newMap ()
{
	MapEditorUndo();
	doClear();
}

void IMapEditorDocument::setState (const State& state)
{
	_map = state.map;
	_settings = state.settingsMap;
	_startPositions = state.startPositions;
	_mapName = state.mapName;
	setMapDimensions(string::toInt(_settings[msn::WIDTH]), string::toInt(_settings[msn::HEIGHT]));
	_highlightItem = nullptr;
	onAfterStateRestored();
}

void IMapEditorDocument::undo ()
{
	endUndoStroke();
	if (_undoStates.empty())
		return;
	const State current = captureState();
	setState(_undoStates.back());
	_redoStates.push_back(current);
	_undoStates.pop_back();
}

void IMapEditorDocument::redo ()
{
	endUndoStroke();
	if (_redoStates.empty())
		return;
	const State current = captureState();
	setState(_redoStates.back());
	_undoStates.push_back(current);
	_redoStates.pop_back();
}

bool IMapEditorDocument::isDirty () const
{
	return _lastSave != _undoStates.size();
}

void IMapEditorDocument::setSetting (const std::string& key, const std::string& value)
{
	_settings[key] = value;
}

std::string IMapEditorDocument::getSetting (const std::string& key, const std::string& fallback) const
{
	const auto i = _settings.find(key);
	if (i == _settings.end() || i->second.empty())
		return fallback;
	return i->second;
}

void IMapEditorDocument::setFileName (const std::string& fileName)
{
	_fileName = fileName;
}

void IMapEditorDocument::setMapName (const std::string& mapName)
{
	_mapName = mapName;
}

void IMapEditorDocument::setMapDimensions (int mapWidth, int mapHeight)
{
	_mapWidth = std::max(MIN_WIDTH, std::min(160, mapWidth));
	_mapHeight = std::max(MIN_HEIGHT, std::min(120, mapHeight));
	setSetting(msn::WIDTH, string::toString(_mapWidth));
	setSetting(msn::HEIGHT, string::toString(_mapHeight));
}

void IMapEditorDocument::setTheme (const ThemeType& theme)
{
	_theme = &theme;
	setSetting(msn::THEME, theme.name);
}

void IMapEditorDocument::setSprite (const SpriteDefPtr& spriteDef)
{
	_activeSprite = spriteDef;
	_activeEntityType = nullptr;
	_activeAngle = spriteDef ? spriteDef->angle : 0;
	_activeEntityRight = true;
	_activeLayer = spriteDef ? getLayer(spriteDef->type) : LAYER_FOREGROUND;
	_tool = Tool::Paint;
}

void IMapEditorDocument::setEmitterEntity (const EntityType& type)
{
	const SpriteDefPtr& spriteDef = SpriteDefinition::get().getFromEntityType(type, getEmitterAnimation(type));
	if (!spriteDef)
		return;
	_activeSprite = spriteDef;
	_activeEntityType = &type;
	_activeAngle = spriteDef->angle;
	_activeEntityRight = true;
	_activeLayer = LAYER_EMITTER;
	_tool = Tool::Paint;
}

void IMapEditorDocument::setActiveEntityRight (bool right)
{
	_activeEntityRight = right;
}

void IMapEditorDocument::rotateBrush ()
{
	if (_activeSprite && _activeSprite->rotateable) {
		_activeAngle += _activeSprite->rotateable;
		_activeAngle %= 360;
	}
}

void IMapEditorDocument::setSelectedGrid (gridCoord x, gridCoord y)
{
	_selectedGridX = x;
	_selectedGridY = y;
}

void IMapEditorDocument::setPlayerPosition (gridCoord gridX, gridCoord gridY)
{
	const IMap::StartPosition p { string::toString(gridX), string::toString(gridY) };
	for (const IMap::StartPosition& position : _startPositions) {
		if (position._x == p._x && position._y == p._y)
			return;
	}
	_startPositions.push_back(p);
}

bool IMapEditorDocument::isOverlapping (gridCoord gridX, gridCoord gridY, const MapEditorTileItem& item) const
{
	return isOverlapping(gridX, gridY, 1.0f, 1.0f, item);
}

bool IMapEditorDocument::isOverlapping (gridCoord gridX, gridCoord gridY, gridSize width, gridSize height,
		const MapEditorTileItem& item) const
{
	const vec2& itemSize = item.getSize(false);
	const gridSize itemW = itemSize.x - 2.0f * EPSILON;
	const gridSize itemH = itemSize.y - 2.0f * EPSILON;
	const gridCoord itemX = item.gridX + item.getX(false) + EPSILON;
	const gridCoord itemY = item.gridY + item.getY(false) + EPSILON;
	if (itemX + itemW <= gridX || itemX >= gridX + width)
		return false;
	if (itemY + itemH <= gridY || itemY >= gridY + height)
		return false;
	return true;
}

bool IMapEditorDocument::isOverlapping (const MapEditorTileItem& item1, const MapEditorTileItem& item2) const
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
		if (item2.layer != LAYER_SOLID)
			return false;
		break;
	default:
		break;
	}
	const vec2& size = item1.getSize(true);
	const gridCoord x = item1.gridX + item1.getX(true) + EPSILON;
	const gridCoord y = item1.gridY + item1.getY(true) + EPSILON;
	return isOverlapping(x, y, size.x - 2.0f * EPSILON, size.y - 2.0f * EPSILON, item2);
}

bool IMapEditorDocument::checkTileHit (const MapEditorTileItem& tileItem, bool remove)
{
	for (auto item = _map.begin(); item != _map.end();) {
		if (!isOverlapping(tileItem, *item)) {
			++item;
			continue;
		}
		if (remove) {
			if (_highlightItem == &(*item))
				_highlightItem = nullptr;
			item = _map.erase(item);
			continue;
		}
		return true;
	}
	return false;
}

bool IMapEditorDocument::placeTileItem (const MapEditorTileItem& item, bool overwrite)
{
	if (!item.def || item.gridX >= _mapWidth || item.gridY >= _mapHeight)
		return false;
	if (overwrite) {
		// Already have an identical tile here - treat as success without mutating.
		for (const MapEditorTileItem& existing : _map) {
			if (existing == item)
				return true;
		}
	}
	const bool hit = checkTileHit(item, overwrite);
	if (hit && !overwrite)
		return false;
	_map.push_back(item);
	_map.sort(mapEditorLayerSort);
	return true;
}

bool IMapEditorDocument::canPlaceTileItem (const MapEditorTileItem& item) const
{
	return true;
}

bool IMapEditorDocument::placeBrushItem (bool overwrite)
{
	if (!_activeSprite)
		return false;
	if (_activeEntityType != nullptr && isPlayerType(*_activeEntityType)) {
		setPlayerPosition(_selectedGridX, _selectedGridY);
		return true;
	}

	MapEditorTileItem item;
	item.def = _activeSprite;
	item.entityType = _activeEntityType;
	item.amount = _activeEntityType ? _emitterAmount : 0;
	item.delay = _activeEntityType ? _emitterDelay : 0;
	item.gridX = _selectedGridX;
	item.gridY = _selectedGridY;
	item.layer = _activeEntityType ? LAYER_EMITTER : _activeLayer;
	item.angle = _activeAngle;
	item.mapTile = _activeEntityType == nullptr && isMapTileType(_activeSprite->type);
	if (!canPlaceTileItem(item))
		return false;
	return placeTileItem(item, overwrite);
}

bool IMapEditorDocument::paintAtSelection (bool overwrite, bool recordUndo)
{
	if (!_activeSprite)
		return false;
	if (_tool == Tool::Erase)
		return eraseAtSelection(recordUndo);
	if (_tool == Tool::Pick) {
		pickAtSelection();
		return true;
	}
	if (recordUndo)
		MapEditorUndo();
	return placeBrushItem(overwrite);
}

bool IMapEditorDocument::eraseAtSelection (bool recordUndo)
{
	if (recordUndo)
		MapEditorUndo();
	const std::string xStr = string::toString(_selectedGridX);
	const std::string yStr = string::toString(_selectedGridY);
	for (auto i = _startPositions.begin(); i != _startPositions.end(); ++i) {
		if (i->_x == xStr && i->_y == yStr) {
			_startPositions.erase(i);
			return true;
		}
	}
	setHighlightFromSelection();
	if (_highlightItem == nullptr)
		return false;
	auto i = std::find(_map.begin(), _map.end(), *_highlightItem);
	if (i == _map.end())
		return false;
	_map.erase(i);
	_highlightItem = getSelectedTile();
	return true;
}

void IMapEditorDocument::pickAtSelection ()
{
	setHighlightFromSelection();
	if (_highlightItem == nullptr)
		return;
	if (_highlightItem->entityType != nullptr) {
		setEmitterEntity(*_highlightItem->entityType);
		_emitterAmount = _highlightItem->amount;
		_emitterDelay = _highlightItem->delay;
		_activeAngle = _highlightItem->angle;
		return;
	}
	setSprite(_highlightItem->def);
	_activeAngle = _highlightItem->angle;
}

void IMapEditorDocument::deleteSelection ()
{
	eraseAtSelection();
}

MapEditorTileItem* IMapEditorDocument::getSelectedTile ()
{
	for (int layer = LAYER_EMITTER; layer != LAYER_NONE; --layer) {
		if (!isLayerActive(layer))
			continue;
		for (MapEditorTileItem& item : _map) {
			if (item.layer != static_cast<MapEditorLayer>(layer))
				continue;
			if (!isOverlapping(_selectedGridX, _selectedGridY, item))
				continue;
			return &item;
		}
	}
	return nullptr;
}

void IMapEditorDocument::setHighlightFromSelection ()
{
	_highlightItem = getSelectedTile();
}

bool IMapEditorDocument::isLayerActive (int layer) const
{
	return (_layerMask & (1 << layer)) != 0;
}

void IMapEditorDocument::toggleLayer (MapEditorLayer layer)
{
	_layerMask ^= (1 << layer);
}

void IMapEditorDocument::shift (int shiftX, int shiftY)
{
	if (_mapWidth + shiftX < MIN_WIDTH || _mapHeight + shiftY < MIN_HEIGHT)
		return;
	MapEditorUndo();
	setMapDimensions(_mapWidth + shiftX, _mapHeight + shiftY);
	for (MapEditorTileItem& item : _map) {
		item.gridX += shiftX;
		item.gridY += shiftY;
	}
}

void IMapEditorDocument::resizeMap (int mapWidth, int mapHeight)
{
	MapEditorUndo();
	setMapDimensions(mapWidth, mapHeight);
}

bool IMapEditorDocument::shouldSaveTile (const MapEditorTileItem& tile) const
{
	return tile.entityType == nullptr;
}

bool IMapEditorDocument::shouldSaveEmitter (const MapEditorTileItem& tile) const
{
	return tile.entityType != nullptr;
}

void IMapEditorDocument::prepareContextForSaving (IMapContext& ctx)
{
	setMapDimensions(_mapWidth, _mapHeight);
	MapEditorTileItems map = _map;
	map.sort();

	ctx.setSettings(_settings);
	ctx.setStartPositions(_startPositions);
	ctx.setTitle(_mapName);

	std::vector<MapTileDefinition> definitions;
	std::vector<EmitterDefinition> emitters;
	for (const MapEditorTileItem& item : map) {
		if (item.gridX >= _mapWidth || item.gridY >= _mapHeight)
			continue;
		if (shouldSaveTile(item))
			definitions.emplace_back(item.gridX, item.gridY, item.def, item.angle);
		else if (shouldSaveEmitter(item))
			emitters.emplace_back(item.gridX, item.gridY, *item.entityType, item.amount, item.delay, item.settings);
	}
	ctx.setMapTileDefinitions(definitions);
	ctx.setEmitterDefinitions(emitters);
}

bool IMapEditorDocument::save ()
{
	if (_fileName.empty())
		return false;
	std::unique_ptr<IMapContext> ctx(createContext(_fileName));
	prepareContextForSaving(*ctx);
	_lastMap->setValue(_fileName);
	_mapManager.loadMaps();
	_lastSave = _undoStates.size();
	return ctx->save();
}

bool IMapEditorDocument::saveAndPlay ()
{
	if (!save())
		return false;
	Commands.executeCommandLine(CMD_MAP_START " " + _fileName);
	return true;
}

void IMapEditorDocument::loadFromContext (IMapContext& ctx)
{
	ctx.load(true);
	setFileName(ctx.getName());
	setMapName(ctx.getTitle());
	_lastMap->setValue(ctx.getName());

	for (const auto& setting : ctx.getSettings())
		setSetting(setting.first, setting.second);
	_startPositions = ctx.getStartPositions();
	setMapDimensions(string::toInt(_settings[msn::WIDTH]), string::toInt(_settings[msn::HEIGHT]));

	for (const MapTileDefinition& tile : ctx.getMapTileDefinitions()) {
		MapEditorTileItem item;
		item.def = tile.spriteDef;
		item.gridX = tile.x;
		item.gridY = tile.y;
		item.layer = getLayer(tile.spriteDef->type);
		item.angle = tile.angle;
		item.mapTile = isMapTileType(tile.spriteDef->type);
		if (!placeTileItem(item, false))
			Log::error(LOG_UI, "could not place tile %s at %f:%f", tile.spriteDef->id.c_str(), tile.x, tile.y);
	}
	for (const EmitterDefinition& emitter : ctx.getEmitterDefinitions()) {
		const EntityType& entityType = *emitter.type;
		const SpriteDefPtr def = SpriteDefinition::get().getFromEntityType(entityType, getEmitterAnimation(entityType));
		if (!def) {
			Log::error(LOG_UI, "could not get sprite for emitter %s", entityType.name.c_str());
			continue;
		}
		MapEditorTileItem item;
		item.def = def;
		item.entityType = &entityType;
		item.amount = emitter.amount;
		item.delay = emitter.delay;
		item.gridX = emitter.x;
		item.gridY = emitter.y;
		item.layer = LAYER_EMITTER;
		item.settings = emitter.settings;
		item.mapTile = false;
		if (!placeTileItem(item, false))
			Log::error(LOG_UI, "could not place emitter %s", entityType.name.c_str());
	}
}

bool IMapEditorDocument::load (const std::string& mapName)
{
	{
		MapEditorUndo();
		doClear();
		std::unique_ptr<IMapContext> ctx(createContext(mapName));
		loadFromContext(*ctx);
	}
	_lastSave = _undoStates.size();
	return true;
}

void IMapEditorDocument::loadLast ()
{
	if (_lastMap && !_lastMap->getValue().empty())
		load(_lastMap->getValue());
}
