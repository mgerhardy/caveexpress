#include "ui/nodes/IUINodeMapEditor.h"
#include "ui/UI.h"
#include "common/ConfigManager.h"
#include "common/CommandSystem.h"
#include "common/SpriteDefinition.h"
#include "common/TextureDefinition.h"
#include "common/Log.h"
#include "common/Commands.h"
#include "common/KeyValueParser.h"
#include "common/FileSystem.h"
#include "common/MapManager.h"
#include "common/IMapContext.h"
#include <ctime>
#include <SDL.h>

namespace {
const float SCALE_STEP = 0.05f;
const uint32_t INITIAL_SCROLL_STEP_DELAY = 500U;
}

bool TileItem::operator== (const TileItem &other) const
{
	if (gridX != other.gridX)
		return false;
	if (gridY != other.gridY)
		return false;
	if (layer != other.layer)
		return false;
	if (def.get() != other.def.get())
		return false;
	if (entityType != other.entityType)
		return false;

	return true;
}

gridCoord TileItem::getX (bool useShape) const
{
	if (useShape && def->hasShape()) {
		return def->getMins().x;
	}
	const vec2 size = getSize();
	if (size.x <= 1.0f + EPSILON) {
		// we start at 0.0f for x
		return 0.0f;
	}
	float integralPart;
	const float frac = ::modff(size.x, &integralPart);
	return -frac;
	//return -(std::max(0.0f, integralPart - 1.0f) - frac);
}

gridCoord TileItem::getY (bool useShape) const
{
	if (_editor->isMapTile(def->type)) {
		return 0.0f;
	}
	if (useShape && def->hasShape()) {
		const float y = def->getMins().y;
		if (_editor->isMapTile(def->type)) {
			return def->height - y;
		}

		return y;
	}
	const vec2 size = getSize();
	if (size.y <= 1.0f + EPSILON) {
		return 1.0f - size.y;
	}
	float integralPart;
	const float frac = ::modff(size.y, &integralPart);
	return -frac;
	//return -(std::max(0.0f, integralPart - 1.0f) - frac);
}

vec2 TileItem::getSize (bool useShape) const
{
	const SpriteType &type = def->type;
	if (useShape && def->hasShape()) {
		return def->calculateSizeFromShapeData();
	} else if (_editor->isMapTile(type)) {
		return vec2(def->width, def->height);
	} else if (entityType) {
		return vec2(entityType->width, entityType->height);
	}

	return vec2(def->width, def->height);
}

bool TileItem::operator< (const TileItem &other) const
{
	if (gridX < other.gridX)
		return true;
	else if (gridX > other.gridX)
		return false;

	if (gridY < other.gridY)
		return true;
	else if (gridY > other.gridY)
		return false;

	return false;
}

StateChecker::StateChecker(IUINodeMapEditor *editor, const TileItems& map, const IMap::SettingsMap& settings,
		const IMap::StartPositions& startPositions, const std::string& mapName, int mapWidth, int mapHeight) :
		_editor(editor), _map(map), _settings(settings), _startPositions(startPositions), _mapName(mapName), _mapWidth(mapWidth), _mapHeight(mapHeight)
{
}

bool StateChecker::checkAndStoreDirtyState (const std::string& id, bool dirty)
{
	if (dirty) {
		_editor->_undoStates.push_back(IUINodeMapEditor::State(_map, _settings, _startPositions, _mapName, _mapWidth, _mapHeight));
		_editor->_redoStates.clear();
	}
	return dirty;
}

StateChecker::~StateChecker ()
{
	if (checkAndStoreDirtyState("mapwidth", _mapWidth != _editor->_mapWidth))
		return;

	if (checkAndStoreDirtyState("mapheight", _mapHeight != _editor->_mapHeight))
		return;

	if (checkAndStoreDirtyState("name", _mapName != _editor->_mapName))
		return;

	if (checkAndStoreDirtyState("mapsize", _map.size() != _editor->_map.size()))
		return;

	if (checkAndStoreDirtyState("settingssize", _settings.size() != _editor->_settings.size()))
		return;

	if (checkAndStoreDirtyState("startpositions", _startPositions.size() != _editor->_startPositions.size()))
		return;

	if (checkAndStoreDirtyState("settings", !std::equal(_settings.begin(), _settings.end(), _editor->_settings.begin())))
		return;

	if (checkAndStoreDirtyState("map", !std::equal(_map.begin(), _map.end(), _editor->_map.begin())))
		return;
}

IUINodeMapEditor::IUINodeMapEditor (IFrontend *frontend, IMapManager& mapManager) :
		UINode(frontend, "mapeditor"), _activeSpriteAngle(0), _activeEntityType(0), _activeEntityTypeRight(true), _selectedGridX(
				0.0f), _selectedGridY(0.0f), _gridScrollX(0), _gridScrollY(0), _activeLayer(LAYER_FOREGROUND), _buttonPressed(
				0), _renderGrid(true), _mapManager(mapManager), _theme(&ThemeTypes::ROCK), _highlightItem(
				nullptr), _moveTileHorizontally(false), _scrollX(0), _scrollY(0), _lastScrollUpdate(0U), _nextScrollDelta(
						INITIAL_SCROLL_STEP_DELAY), _layerMask(0xFFFFFFFF), _lastSave(0u)
{
	_font = getFont();
	Vector4Set(colorBlack, _fontColor);
	_tileWidth = loadTexture("tile-reference")->getWidth();
	_renderBorder = true;
	_lastMap = Config.getConfigVar("editor-lastmap");

	doClear();

	Commands.registerCommand(CMD_LOADMAP, bindFunction(IUINodeMapEditor, loadMap));
}

IUINodeMapEditor::~IUINodeMapEditor ()
{
	_editorListeners.clear();
}

void IUINodeMapEditor::loadMap (const ICommand::Args& args)
{
	if (args.size() != 1) {
		Log::error(LOG_CLIENT, "no map given");
		return;
	}

	if (args[0].empty()) {
		Log::error(LOG_CLIENT, "invalid map given");
		return;
	}

	UI::get().pushRoot(UI_WINDOW_EDITOR);
	Log::error(LOG_CLIENT, "map %s is loading", args[0].c_str());
	load(args[0]);
}

inline int IUINodeMapEditor::getTileWidth () const
{
	return _tileWidth * _scale;
}

inline int IUINodeMapEditor::getTileHeight () const
{
	return std::max(1.0f, _tileWidth * _scale + 0.5f);
}

inline int IUINodeMapEditor::getScreenMapGridMaxWidth () const
{
	const int maxVisible = static_cast<int>(ceil(getRenderWidth() / static_cast<double>(getTileWidth())));
	return maxVisible;
}

inline int IUINodeMapEditor::getScreenMapGridMaxHeight () const
{
	const int maxVisible = static_cast<int>(ceil(getRenderHeight() / static_cast<double>(getTileHeight())));
	return maxVisible;
}

inline int IUINodeMapEditor::getScreenMapGridWidth () const
{
	const int tiles = std::min(_mapWidth, getScreenMapGridMaxWidth());
	return tiles;
}

inline int IUINodeMapEditor::getScreenMapGridHeight () const
{
	const int tiles = std::min(_mapHeight, getScreenMapGridMaxHeight());
	return tiles;
}

void IUINodeMapEditor::renderBorder (const TileItem& item, int x, int y, const Color& color, bool fullSize, bool inner) const
{
	const bool useShape = !fullSize;
	const vec2 size = item.getSize(useShape);
	const float tileWidth = getTileWidth();
	const float tileHeight = getTileHeight();
	const int rx = getRenderX() + x + ((item.gridX - (gridCoord)_gridScrollX + item.getX(useShape)) * tileWidth);
	const int ry = getRenderY() + y + ((item.gridY - (gridCoord)_gridScrollY + item.getY(useShape)) * tileHeight);
	const int rw = size.x * tileWidth;
	const int rh = size.y * tileHeight;
	// TODO: angle is not taken into account here
	if (inner) {
		_frontend->renderRect(rx + 1, ry + 1, rw - 2, rh - 2, color);
	} else {
		_frontend->renderRect(rx, ry, rw, rh, color);
	}
}

void IUINodeMapEditor::render (int x, int y) const
{
	enableScissor(x, y);

	UINode::render(x, y);

	for (TileItemsConstIter pItem = _map.begin(); pItem != _map.end(); ++pItem) {
		const TileItem& item = *pItem;
		renderSprite(item, x, y);
		if (_highlightItem == &item) {
			renderBorder(item, x, y, colorYellow, true, false);
		}
	}

	renderPlayer(x, y);
	if (_renderGrid)
		renderGrid(x, y);

	disableScissor();

	renderScrollbars(x, y);

	if (!hasFocus())
		return;

	if (_activeSpriteDefition) {
		const TileItem item = { this, _activeSpriteDefition, _activeEntityType, 0, 0, _selectedGridX, _selectedGridY, LAYER_NONE, _activeSpriteAngle, "" };
		renderSprite(item, x, y, 0.6f);
		renderBorder(item, x, y, colorGreen, false, true);
	}

	renderHighlightItem(x, y);
}

void IUINodeMapEditor::renderHighlightItem (int x, int y) const
{
}

// FIXME: does not work right now
void IUINodeMapEditor::renderScrollbars (int x, int y) const
{
	const int scrollBarPadding = -3;
	const int scrollBarSize = std::abs(scrollBarPadding);
	const Color colorBar = { 0.7f, 0.7f, 0.7f, 0.4f };
	const Color color = { 0.8f, 0.8f, 0.8f, 1.0f };
	const int visibleMapGridWidth = getRenderWidth() / _tileWidth;
	const int visibleMapGridHeight = getRenderHeight() / _tileWidth;
	if (_mapHeight > visibleMapGridHeight) {
		const int xBarRight = x + getRenderX() + getRenderWidth() + scrollBarPadding;
		const int yBarRight = y + getRenderY();
		const int wBarRight = scrollBarSize;
		const int hBarRight = getRenderHeight();
		renderFilledRect(xBarRight, yBarRight, wBarRight, hBarRight, colorBar);

		const float aspect = visibleMapGridHeight / (float)_mapHeight;
		const int yRight = yBarRight + _gridScrollY * _tileWidth * aspect;
		const int hRight = visibleMapGridHeight * _tileWidth * aspect;
		renderFilledRect(xBarRight, yRight, wBarRight, hRight, color);
	}

	if (_mapWidth > visibleMapGridWidth) {
		const int xBarBottom = x + getRenderX();
		const int yBarBottom = y + getRenderY() + getRenderHeight() + scrollBarPadding;
		const int wBarBottom = getRenderWidth();
		const int hBarBottom = scrollBarSize;
		renderFilledRect(xBarBottom, yBarBottom, wBarBottom, hBarBottom, colorBar);

		const float aspect = visibleMapGridWidth / (float)_mapWidth;
		const int xBottom = xBarBottom + _gridScrollX * _tileWidth * aspect;
		const int wBottom = visibleMapGridWidth * _tileWidth * aspect;
		renderFilledRect(xBottom, yBarBottom, wBottom, hBarBottom, color);
	}
}

void IUINodeMapEditor::renderGrid (int x, int y) const
{
	const float tileWidth = getTileWidth();
	const float tileHeight = getTileHeight();
	const Color gridColor = { 0.8f, 0.8f, 0.8f, 0.2f };
	const int screenMapGridWidth = getScreenMapGridWidth();
	const int screenMapGridHeight = getScreenMapGridHeight();
	{
		const int _y = y + getRenderY();
		const int h = _y + screenMapGridHeight * tileHeight;
		for (int i = 0; i <= screenMapGridWidth; ++i) {
			const int _x = x + getRenderX() + i * tileWidth;
			renderLine(_x, _y, _x, h, gridColor);
		}
	}
	{
		const int _x = x + getRenderX();
		const int w = _x + screenMapGridWidth * tileWidth;
		for (int i = 0; i <= screenMapGridHeight; ++i) {
			const int _y = y + getRenderY() + i * tileHeight;
			renderLine(_x, _y, w, _y, gridColor);
		}
	}
}

void IUINodeMapEditor::renderSprite (const TileItem& item, const int x, const int y, float alpha) const
{
	const SpritePtr& sprite = UI::get().loadSprite(item.def->id);
	const vec2 size = item.getSize();
	const float tileWidth = getTileWidth();
	const float tileHeight = getTileHeight();
	const int rx = getRenderX() + x + ((item.gridX - (gridCoord)_gridScrollX + item.getX()) * tileWidth);
	const int ry = getRenderY() + y + ((item.gridY - (gridCoord)_gridScrollY + item.getY()) * tileHeight);
	const int rw = size.x * tileWidth;
	const int rh = size.y * tileHeight;

	for (Layer layer = LAYER_BACK; layer < MAX_LAYERS; ++layer) {
		sprite->render(_frontend, layer, rx, ry, rw, rh, item.angle, alpha);
	}
}

gridCoord IUINodeMapEditor::getGridX () const
{
	const int nodePos = _focusMouseX - getRenderX();
	const float tileWidth = getTileWidth();
	const int pos = nodePos / tileWidth;
	return pos + _gridScrollX;
}

gridCoord IUINodeMapEditor::getGridY () const
{
	const int nodePos = _focusMouseY - getRenderY();
	const float tileHeight = getTileHeight();
	const int pos = nodePos / tileHeight;
	return pos + _gridScrollY;
}

bool IUINodeMapEditor::onMouseButtonRelease (int32_t x, int32_t y, unsigned char button)
{
	_buttonPressed = 0;

	switch (button) {
	case SDL_BUTTON_LEFT: {
		if (!_activeSpriteDefition)
			return false;
		Undo();
		placeTileItem(true);
		return true;
	}
	case SDL_BUTTON_MIDDLE: {
		selectPlacedTile();
		return true;
	}
	case SDL_BUTTON_RIGHT: {
		Undo();
		deleteItem();
		return true;
	}
	}

	return false;
}

void IUINodeMapEditor::removeFocus ()
{
	UINode::removeFocus();
	_buttonPressed = 0;
}

bool IUINodeMapEditor::onMouseButtonPress (int32_t x, int32_t y, unsigned char button)
{
	_buttonPressed = button;

	switch (button) {
	case SDL_BUTTON_LEFT: {
		if (!_activeSpriteDefition)
			return false;
		return true;
	}
	}

	return false;
}

bool IUINodeMapEditor::isLayerActive (int layer) const
{
	return _layerMask & (1 << layer);
}

void IUINodeMapEditor::toggleLayer (MapEditorLayer layer)
{
	_layerMask ^= (1 << layer);
}

TileItem* IUINodeMapEditor::getSelectedTile ()
{
	for (int layer = LAYER_EMITTER; layer != LAYER_NONE; --layer) {
		if (!isLayerActive(layer))
			continue;
		for (TileItemsIter item = _map.begin(); item != _map.end(); ++item) {
			if (item->layer != static_cast<MapEditorLayer>(layer))
				continue;
			if (!isOverlapping(_selectedGridX, _selectedGridY, *item))
				continue;
			return &(*item);
		}
	}
	return nullptr;
}

void IUINodeMapEditor::moveTile (bool right)
{
	if (_highlightItem == nullptr)
		return;
	// you can't shift everything
	const SpriteType &type = _highlightItem->def->type;
	if (!isMapTile(type))
		return;
	const gridCoord shiftAmount = right ? 0.1f : -0.1f;
	Undo();
	const int checkCoord = _highlightItem->gridX;
	const int check = _highlightItem->gridX + shiftAmount;
	// prevent from shifting into other tile ranges
	if (check == checkCoord)
		_highlightItem->gridX += shiftAmount;
}

void IUINodeMapEditor::deleteItem ()
{
	if (_highlightItem == nullptr)
		return;
	const SpriteType &type = _highlightItem->def->type;
	notifyTileRemoved(_highlightItem->def);
	TileItemsIter i = std::find(_map.begin(), _map.end(), *_highlightItem);
	_map.erase(i);
	_highlightItem = getSelectedTile();
}

bool IUINodeMapEditor::onKeyRelease (int32_t key)
{
	if (key == SDLK_LALT || key == SDLK_RALT) {
		_moveTileHorizontally = false;
		return true;
	}
	switch (key) {
	case SDLK_LEFT:
	case SDLK_RIGHT:
		_scrollX = 0;
		if (_scrollY == 0)
			_nextScrollDelta = INITIAL_SCROLL_STEP_DELAY;
		return true;
	case SDLK_DOWN:
	case SDLK_UP:
		_scrollY = 0;
		if (_scrollX == 0)
			_nextScrollDelta = INITIAL_SCROLL_STEP_DELAY;
		return true;
	}
	return false;
}

bool IUINodeMapEditor::onKeyPress (int32_t key, int16_t modifier)
{
	if (key == SDLK_LALT || key == SDLK_RALT) {
		_moveTileHorizontally = true;
		return true;
	}

	if (key == SDLK_z) {
		if (modifier & KMOD_CTRL)
			undo();
		else
			return false;

		updateScrolling();
		return true;
	} else if (key == SDLK_y) {
		if (modifier & KMOD_CTRL)
			redo();
		else
			return false;

		updateScrolling();
		return true;
	}

	Undo();

	const int oldWidth = _mapWidth;
	const int oldHeight = _mapHeight;

	switch (key) {
	case SDLK_PLUS :
	case SDLK_KP_PLUS:
		if (modifier & KMOD_SHIFT)
			++_mapWidth;
		else
			++_mapHeight;
		break;
	case SDLK_MINUS:
	case SDLK_KP_MINUS:
		if (modifier & KMOD_SHIFT)
			--_mapWidth;
		else
			--_mapHeight;
		// TODO: update scaling
		break;
	case SDLK_LEFT:
		if (modifier & KMOD_SHIFT)
			shift(-1, 0);
		else
			_scrollX = -1;
		break;
	case SDLK_RIGHT:
		if (modifier & KMOD_SHIFT)
			shift(1, 0);
		else
			_scrollX = 1;
		break;
	case SDLK_UP:
		if (modifier & KMOD_SHIFT)
			shift(0, -1);
		else
			_scrollY = -1;
		break;
	case SDLK_DOWN:
		if (modifier & KMOD_SHIFT)
			shift(0, 1);
		else
			_scrollY = 1;
		break;
	case SDLK_s:
		save();
		break;
	case SDLK_SPACE:
		if (_activeSpriteDefition && _activeSpriteDefition->rotateable) {
			Log::info(LOG_CLIENT, "rotate %s by %i", _activeSpriteDefition->id.c_str(), _activeSpriteDefition->rotateable);
			_activeSpriteAngle += _activeSpriteDefition->rotateable;
			_activeSpriteAngle %= 360;
		}
		break;
	default:
		return false;
	}

	_mapWidth = clamp(_mapWidth, MIN_WIDTH, 160);
	_mapHeight = clamp(_mapHeight, MIN_HEIGHT, 120);

	updateScrolling();

	if (oldWidth != _mapWidth || oldHeight != _mapHeight)
		setMapDimensions(_mapWidth, _mapHeight);

	return true;
}

void IUINodeMapEditor::update (uint32_t deltaTime)
{
	UINode::update(deltaTime);
	if (_lastScrollUpdate <= 0) {
		_lastScrollUpdate = _nextScrollDelta;
		_nextScrollDelta /= 2;
		_gridScrollX += _scrollX;
		_gridScrollY += _scrollY;
		updateScrolling();
	} else {
		_lastScrollUpdate -= deltaTime;
	}
}

void IUINodeMapEditor::updateScrolling ()
{
	const int visibleWidth = getScreenMapGridWidth();
	const int visibleHeight = getScreenMapGridHeight();
	const int halfVisibleX = visibleWidth * getTileWidth() - getRenderWidth() > 0 ? 1 : 0;
	const int halfVisibleY = visibleHeight * getTileHeight() - getRenderHeight() > 0 ? 1 : 0;
	const int maxGridScrollY = _mapHeight - visibleHeight + halfVisibleX;
	const int maxGridScrollX = _mapWidth - visibleWidth + halfVisibleY;

	_gridScrollX = clamp(_gridScrollX, 0, maxGridScrollX);
	_gridScrollY = clamp(_gridScrollY, 0, maxGridScrollY);

	_selectedGridX = getGridX();
	_selectedGridY = getGridY();
}

void IUINodeMapEditor::onMouseMotion (int32_t x, int32_t y, int32_t relX, int32_t relY)
{
	UINode::onMouseMotion(x, y, relX, relY);

	const int oldSlectedGridX = _selectedGridX;
	const int oldSelectedGridY = _selectedGridY;
	_selectedGridX = getGridX();
	_selectedGridY = getGridY();

	switch (_buttonPressed) {
	case SDL_BUTTON_LEFT: {
		Undo();
		placeTileItem(true);
		return;
	}
	case SDL_BUTTON_RIGHT: {
		Undo();
		deleteItem();
		return;
	}
	}

	const bool gridPosChanged = oldSlectedGridX != _selectedGridX || oldSelectedGridY != _selectedGridY;
	if (gridPosChanged) {
		_highlightItem = getSelectedTile();
	}

	if (_moveTileHorizontally) {
		moveTile(relX > 0);
	}
}

void IUINodeMapEditor::fitView ()
{
	_gridScrollX = _gridScrollY = 0;
	_scale = 1.0f / static_cast<float>(_tileWidth);
	scale(-0.001f);
}

void IUINodeMapEditor::scale (float scaleFactor)
{
	_scale = std::max(0.0001f, _scale + scaleFactor);
	const float maxScale = 256.0f / _tileWidth;
	if (_scale > maxScale) {
		_scale = maxScale;
	} else if (scaleFactor < 0.0f) {
		const float absMinScale = 1.0f / static_cast<float>(_tileWidth);
		if (_scale < absMinScale)
			_scale = absMinScale;
		const int visibleWidth = getScreenMapGridMaxWidth();
		const int visibleHeight = getScreenMapGridMaxHeight();
		// if we are seeing more than we need on the current scale level, we set the scale to the max needed scale to see everything
		if (_mapHeight <= visibleHeight && _mapWidth <= visibleWidth) {
			const int minTileWidth = std::min(getRenderWidth() / static_cast<float>(_mapWidth), getRenderHeight() / static_cast<float>(_mapHeight));
			const float minScale = minTileWidth / static_cast<float>(_tileWidth);
			_scale = minScale;
		}
	}
}

bool IUINodeMapEditor::onMouseWheel (int32_t x, int32_t y)
{
	scale(y * SCALE_STEP);
	updateScrolling();
	return true;
}

void IUINodeMapEditor::selectPlacedTile ()
{
	if (_highlightItem == nullptr)
		return;

	const EntityType* t = _highlightItem->entityType;
	if (t != nullptr && !t->isNone()) {
		setEmitterEntity(*t);
	} else if (_highlightItem->def) {
		setSprite(_highlightItem->def);
		notifySelectionChange(_highlightItem->def);
	}
	_activeSpriteAngle = _highlightItem->angle;
	_activeEntityTypeRight = false;
}

bool IUINodeMapEditor::placeTileItem (bool overwrite)
{
	// nothing selected yet
	if (!_activeSpriteDefition)
		return false;

	if (_activeEntityType) {
		if (isPlayer(*_activeEntityType)) {
			setPlayerPosition(_selectedGridX, _selectedGridY);
			return true;
		}
		return placeEmitter(_activeSpriteDefition, _activeEntityType, _selectedGridX, _selectedGridY, 1, 0, false, _activeSpriteAngle, "");
	}
	const SpriteType& type = _activeSpriteDefition->type;
	return placeTileItem(_activeSpriteDefition, _activeEntityType, _selectedGridX, _selectedGridY, _activeLayer, overwrite, _activeSpriteAngle);
}

bool IUINodeMapEditor::placeEmitter (const SpriteDefPtr& def, const EntityType* entityType, gridCoord gridX,
		gridCoord gridY, int emitterAmount, int emitterDelay, bool overwrite, EntityAngle angle, const std::string& settings)
{
	const TileItem item = { this, def, entityType, emitterAmount, emitterDelay, gridX, gridY, LAYER_EMITTER, angle, settings };
	return placeTileItem(item, overwrite);
}

bool IUINodeMapEditor::placeTileItem (const SpriteDefPtr& def, const EntityType* entityType, gridCoord gridX, gridCoord gridY,
		MapEditorLayer layer, bool overwrite, EntityAngle angle)
{
	const TileItem item = { this, def, entityType, 0, 0, gridX, gridY, layer, angle, "" };
	return placeTileItem(item, overwrite);
}

bool IUINodeMapEditor::placeTileItem (const TileItem& item, bool overwrite)
{
	if (!item.def || item.gridX >= _mapWidth || item.gridY >= _mapHeight)
		return false;

	const bool hit = checkTileHit(item, overwrite);
	if (hit && !overwrite)
		return false;

	notifyTilePlaced(item.def);
	_map.push_back(item);
	_map.sort(layerSort);
	return true;
}

bool IUINodeMapEditor::isOverlapping (gridCoord gridX, gridCoord gridY, const TileItem& item) const
{
	return isOverlapping(gridX, gridY, 1.0f, 1.0f, item);
}

bool IUINodeMapEditor::isOverlapping (const TileItem& item1, const TileItem& item2) const
{
	switch (item1.layer) {
	case LAYER_BACKGROUND:
		if (item2.layer == LAYER_FOREGROUND || item2.layer == LAYER_DECORATION || item2.layer == LAYER_EMITTER) {
			return false;
		}
		break;
	case LAYER_FOREGROUND:
		if (item2.layer == LAYER_BACKGROUND || item2.layer == LAYER_DECORATION || item2.layer == LAYER_EMITTER) {
			return false;
		}
		break;
	case LAYER_DECORATION:
	case LAYER_EMITTER:
		if (item2.layer != LAYER_SOLID) {
			return false;
		}
		break;
	case LAYER_SOLID:
	case LAYER_NONE:
	case LAYER_MAX:
		break;
	}
	const bool useShape = true;
	const vec2& size = item1.getSize(useShape);
	const gridCoord x = item1.gridX + item1.getX(useShape) + EPSILON;
	const gridCoord y = item1.gridY + item1.getY(useShape) + EPSILON;
	const gridSize w = size.x - 2.0f * EPSILON;
	const gridSize h = size.y - 2.0f * EPSILON;
	return isOverlapping(x, y, w, h, item2);
}

bool IUINodeMapEditor::isOverlapping (gridCoord gridX, gridCoord gridY, gridSize width, gridSize height, const TileItem& item) const
{
	const bool useShape = false;
	const vec2& itemSize = item.getSize(useShape);
	const gridSize itemW = itemSize.x - 2.0f * EPSILON;
	const gridSize itemH = itemSize.y - 2.0f * EPSILON;
	const gridCoord itemX = item.gridX + item.getX(useShape) + EPSILON;
	const gridCoord itemY = item.gridY + item.getY(useShape) + EPSILON;
	if (itemX + itemW <= gridX)
		return false;
	if (itemX >= gridX + width)
		return false;

	if (itemY + itemH <= gridY)
		return false;
	if (itemY >= gridY + height)
		return false;

	return true;
}

bool IUINodeMapEditor::checkTileHit (const TileItem& tileItem, bool remove)
{
	for (TileItemsIter item = _map.begin(); item != _map.end();) {
		const bool hit = isOverlapping(tileItem, *item);
		if (!hit) {
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

void IUINodeMapEditor::setState (const State& state)
{
	_map = state.map;
	_settings = state.settingsMap;
	_startPositions = state.startPositions;
	setMapName(state.mapName);
	setMapDimensions(string::toInt(_settings[msn::WIDTH]), string::toInt(_settings[msn::HEIGHT]));
}

void IUINodeMapEditor::undo ()
{
	if (_undoStates.empty())
		return;

	const State currentState(_map, _settings, _startPositions, _mapName, _mapWidth, _mapHeight);
	const State& oldState = _undoStates.back();
	setState(oldState);
	_redoStates.push_back(currentState);
	_undoStates.pop_back();
}

void IUINodeMapEditor::redo ()
{
	if (_redoStates.empty())
		return;
	const State currentState(_map, _settings, _startPositions, _mapName, _mapWidth, _mapHeight);
	const State& state = _redoStates.front();
	setState(state);
	_redoStates.erase(_redoStates.begin());
	_undoStates.push_back(currentState);
}

void IUINodeMapEditor::clear ()
{
	Undo();
	doClear();
}

void IUINodeMapEditor::shift (int shiftX, int shiftY)
{
	if (_mapWidth + shiftX < MIN_WIDTH)
		return;
	if (_mapHeight + shiftY < MIN_HEIGHT)
		return;

	Undo();
	setMapDimensions(_mapWidth + shiftX, _mapHeight + shiftY);
	for (TileItemsIter i = _map.begin(); i != _map.end(); ++i) {
		i->gridX += shiftX;
		i->gridY += shiftY;
	}
}

void IUINodeMapEditor::doClear ()
{
	_highlightItem = nullptr;
	_map.clear();
	for (IMap::SettingsMapConstIter i = _settings.begin(); i != _settings.end(); ++i) {
		setSetting(i->first, "");
	}
	_lastSave = 0;
	_settings.clear();
	setFileName("newmap");
	setMapName("A new map");
	setMapDimensions(16, 12);
	_startPositions.clear();
	setSetting(msn::POINTS, string::toString(msdv::POINTS));
	setSetting(msn::REFERENCETIME, string::toString(msdv::REFERENCETIME));
	fitView();
}

bool IUINodeMapEditor::isDirty () const
{
	Log::info(LOG_CLIENT, "%i:%i", (int)_lastSave, (int)_undoStates.size());
	return _lastSave != _undoStates.size();
}

bool IUINodeMapEditor::shouldSaveTile (const TileItem& tile) const
{
	return tile.entityType == nullptr;
}

bool IUINodeMapEditor::shouldSaveEmitter (const TileItem& tile) const
{
	return tile.entityType != nullptr;
}

void IUINodeMapEditor::saveTiles (const FilePtr& file, const TileItems& map) const
{
	for (TileItemsConstIter i = map.begin(); i != map.end(); ++i) {
		if (i->gridX >= _mapWidth || i->gridY >= _mapHeight)
			continue;
		if (shouldSaveTile(*i)) {
			file->appendString("\tmap:addTile(\"");
			file->appendString(i->def->id.c_str());
			file->appendString("\", ");
			file->appendString(string::toString(i->gridX).c_str());
			file->appendString(", ");
			file->appendString(string::toString(i->gridY).c_str());
			if (i->angle != 0) {
				file->appendString(", ");
				file->appendString(string::toString(i->angle).c_str());
			}
			file->appendString(")\n");
		}
	}
	file->appendString("\n");

	bool emitterAdded = false;
	for (TileItemsConstIter i = map.begin(); i != map.end(); ++i) {
		if (i->gridX >= _mapWidth || i->gridY >= _mapHeight)
			continue;
		if (!shouldSaveEmitter(*i))
			continue;
		file->appendString("\tmap:addEmitter(");
		file->appendString("\"");
		file->appendString(i->entityType->name.c_str());
		file->appendString("\", ");
		file->appendString(string::toString(i->gridX).c_str());
		file->appendString(", ");
		file->appendString(string::toString(i->gridY).c_str());
		file->appendString(", ");
		file->appendString(string::toString(std::min(50, i->amount)).c_str());
		file->appendString(", ");
		file->appendString(string::toString(i->delay).c_str());
		file->appendString(", \"");
		file->appendString(i->settings.c_str());
		file->appendString("\")\n");
		emitterAdded = true;
	}

	if (emitterAdded)
		file->appendString("\n");
}

bool IUINodeMapEditor::save ()
{
	// nothing to save here
	if (_undoStates.empty())
		return false;

	TileItems map = _map;
	map.sort();

	const std::string path = FS.getAbsoluteWritePath() + FS.getDataDir() + FS.getMapsDir() + _fileName + ".lua";
	SDL_RWops *rwops = FS.createRWops(path, "wb");
	FilePtr file(new File(rwops, path));

	file->writeString("function getName()\n");
	file->appendString("\treturn \"");
	file->appendString(_mapName.c_str());
	file->appendString("\"\n");
	file->appendString("end\n\n");
	file->appendString("function onMapLoaded()\n");
	file->appendString("end\n\n");
	file->appendString("function initMap()\n");
	file->appendString("\t-- get the current map context\n");
	file->appendString("\tlocal map = Map.get()\n");

	saveTiles(file, map);

	IMap::SettingsMap& settings = _settings;
	file->appendString("\tmap:setSetting(\"");
	file->appendString(msn::WIDTH.c_str());
	file->appendString("\", \"");
	file->appendString(settings[msn::WIDTH].c_str());
	file->appendString("\")\n");;
	file->appendString("\tmap:setSetting(\"");
	file->appendString(msn::HEIGHT.c_str());
	file->appendString("\", \"");
	file->appendString(settings[msn::HEIGHT].c_str());
	file->appendString("\")\n");
	for (IMap::SettingsMapConstIter i = settings.begin(); i != settings.end(); ++i) {
		if (i->first == msn::WIDTH || i->first == msn::HEIGHT)
			continue;
		file->appendString("\tmap:setSetting(\"");
		file->appendString(i->first.c_str());
		file->appendString("\", \"");
		file->appendString(i->second.c_str());
		file->appendString("\")\n");
	}

	for (const IMap::StartPosition& pos : _startPositions) {
		file->appendString("\tmap:addStartPosition(\"");
		file->appendString(pos._x.c_str());
		file->appendString("\", \"");
		file->appendString(pos._y.c_str());
		file->appendString("\")\n");
	}

	file->appendString("end\n");

	Log::info(LOG_GENERAL, "wrote %s", path.c_str());
	_lastMap->setValue(_fileName);
	_mapManager.loadMaps();
	_lastSave = _undoStates.size();
	return true;
}

void IUINodeMapEditor::loadFromContext (IMapContext& ctx)
{
	ctx.load(true);
	setFileName(ctx.getName());
	setMapName(ctx.getTitle());
	_lastMap->setValue(ctx.getName());

	const IMap::SettingsMap& settings = ctx.getSettings();
	for (IMap::SettingsMapConstIter i = settings.begin(); i != settings.end(); ++i) {
		setSetting(i->first, i->second);
	}
	_startPositions = ctx.getStartPositions();
	const int mapWidth = string::toInt(_settings[msn::WIDTH]);
	const int mapHeight = string::toInt(_settings[msn::HEIGHT]);
	setMapDimensions(mapWidth, mapHeight);

	const std::vector<MapTileDefinition>& mapTiles = ctx.getMapTileDefinitions();
	Log::info(LOG_CLIENT, "place %i maptiles", static_cast<int>(mapTiles.size()));
	for (std::vector<MapTileDefinition>::const_iterator i = mapTiles.begin(); i != mapTiles.end(); ++i) {
		const SpriteType& type = i->spriteDef->type;
		const MapEditorLayer layer = getLayer(type);
		if (!placeTileItem(i->spriteDef, nullptr, i->x, i->y, layer, false, i->angle))
			Log::error(LOG_CLIENT, "could not place tile %s at %f:%f", i->spriteDef->id.c_str(), i->x, i->y);
	}
	const std::vector<EmitterDefinition>& emitters = ctx.getEmitterDefinitions();
	Log::info(LOG_CLIENT, "place %i emitters", static_cast<int>(emitters.size()));
	for (std::vector<EmitterDefinition>::const_iterator i = emitters.begin(); i != emitters.end(); ++i) {
		const EntityType& entityType = *i->type;
		const gridCoord x = i->x;
		const gridCoord y = i->y;
		const int amount = i->amount;
		const int delay = i->delay;
		const KeyValueParser s(i->settings);
		const Animation& animation = getEmitterAnimation(entityType);
		const SpriteDefPtr def = SpriteDefinition::get().getFromEntityType(entityType, animation);
		if (!def) {
			Log::error(LOG_CLIENT, "could not get the sprite definition for the entity type: %s", entityType.name.c_str());
			continue;
		}
		if (!placeEmitter(def, &entityType, x, y, amount, delay, false, 0.0f, i->settings))
			Log::error(LOG_CLIENT, "could not place emitter %s at %f:%f", i->type->name.c_str(), x, y);
	}
}

void IUINodeMapEditor::loadLast ()
{
	load(_lastMap->getValue());
}

void IUINodeMapEditor::load (const std::string& mapName)
{
	Log::info(LOG_CLIENT, "mapname: %s", mapName.c_str());
	{
		// we need this scoped because of the undo dtor
		Undo();
		doClear();
		std::unique_ptr<IMapContext> ctx(getContext(mapName));
		loadFromContext(*ctx.get());
	}
	fitView();
	_lastSave = _undoStates.size();
}

void IUINodeMapEditor::addEditorListener (IMapEditorListener *listener)
{
	_editorListeners.push_back(listener);
}

void IUINodeMapEditor::notifyTilePlaced (const SpriteDefPtr& def)
{
	for (std::vector<IMapEditorListener*>::iterator i = _editorListeners.begin(); i != _editorListeners.end(); ++i) {
		(*i)->onTilePlaced(def);
	}
}

void IUINodeMapEditor::notifyNewMap ()
{
	for (std::vector<IMapEditorListener*>::iterator i = _editorListeners.begin(); i != _editorListeners.end(); ++i) {
		(*i)->onNewMap();
	}
}

void IUINodeMapEditor::notifyTileRemoved (const SpriteDefPtr& def)
{
	for (std::vector<IMapEditorListener*>::iterator i = _editorListeners.begin(); i != _editorListeners.end(); ++i) {
		(*i)->onTileRemoved(def);
	}
}

void IUINodeMapEditor::notifySelectionChange (const SpriteDefPtr& def)
{
	for (std::vector<IMapEditorListener*>::iterator i = _editorListeners.begin(); i != _editorListeners.end(); ++i) {
		(*i)->onTileSelected(def);
	}
}

void IUINodeMapEditor::setSetting (const std::string& key, const std::string& value)
{
	_settings[key] = value;
	for (std::vector<IMapEditorListener*>::iterator i = _editorListeners.begin(); i != _editorListeners.end(); ++i) {
		(*i)->onSettingsValueChange(key, value);
	}
}

void IUINodeMapEditor::toggleGrid ()
{
	_renderGrid ^= true;
}

bool IUINodeMapEditor::isActive () const
{
	return true;
}

void IUINodeMapEditor::setFileName (const std::string& fileName)
{
	const std::string old = _fileName;
	_fileName = fileName;
	for (std::vector<IMapEditorListener*>::iterator i = _editorListeners.begin(); i != _editorListeners.end(); ++i) {
		(*i)->onFileNameChange(old, _fileName);
	}
}
