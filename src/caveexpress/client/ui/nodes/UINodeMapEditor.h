#pragma once

#include "engine/client/ui/nodes/UINode.h"
#include "engine/client/sprites/Sprite.h"
#include "engine/client/ui/BitmapFont.h"
#include "engine/common/SpriteDefinition.h"
#include "engine/common/MapSettings.h"
#include "caveexpress/shared/CaveExpressSpriteType.h"
#include "caveexpress/shared/CaveExpressEntityType.h"
#include "caveexpress/shared/CaveExpressAnimation.h"
#include "engine/common/EntityType.h"
#include "engine/common/SpriteType.h"
#include "engine/common/Animation.h"
#include "engine/common/String.h"
#include "engine/common/ConfigVar.h"
#include "engine/common/ICommand.h"
#include "engine/common/IMap.h"
#include <vector>
#include <list>
#include <map>

// forward decl
class IMapContext;
class StateChecker;
class IMapManager;

namespace {
const int MIN_WIDTH = 6;
const int MIN_HEIGHT = 4;
}

enum MapEditorLayer {
	LAYER_NONE, LAYER_BACKGROUND, LAYER_SOLID, LAYER_FOREGROUND, /* can be placed on background tiles */
	LAYER_DECORATION, /* can be placed on background or other decoration tiles */
	LAYER_EMITTER, /* can be placed on background or decoration tiles */
	LAYER_MAX
};

static const std::string MapEditorLayerNames[LAYER_MAX] = {
	"none", "background", "solid", "foreground", "decoration", "emitter"
};

class TileItem {
public:
	SpriteDefPtr def;
	const EntityType* entityType;
	int amount;
	int delay;
	gridCoord gridX; // the grid position
	gridCoord gridY; // the grid position
	MapEditorLayer layer;
	EntityAngle angle;
	// key=value,key=value
	std::string settings;

	bool operator== (const TileItem &other) const
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

	gridCoord getX (bool useShape = false) const
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

	gridCoord getY (bool useShape = false) const
	{
		if (SpriteTypes::isMapTile(def->type)) {
			return 0.0f;
		}
		if (useShape && def->hasShape()) {
			const float y = def->getMins().y;
			if (SpriteTypes::isMapTile(def->type)) {
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

	vec2 getSize (bool useShape = false) const
	{
		const SpriteType &type = def->type;
		if (useShape && def->hasShape()) {
			return def->calculateSizeFromShapeData();
		} else if (SpriteTypes::isMapTile(type)) {
			return vec2(def->width, def->height);
		} else if (entityType) {
			return vec2(entityType->width, entityType->height);
		}

		return vec2(def->width, def->height);
	}

	bool operator< (const TileItem &other) const
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

	inline std::string toString () const
	{
		std::stringstream ss;
		ss << "spritedef: " << def->id << std::endl;
		ss << "entitytype: " << (entityType != nullptr ? entityType->name : "none") << std::endl;
		ss << "emitterAmount: " << amount << std::endl;
		ss << "emitterDelay: " << delay << std::endl;
		ss << "gridX: " << gridX << std::endl;
		ss << "gridY: " << gridY << std::endl;
		ss << "layer: " << layer << std::endl;
		ss << "settings: " << settings << std::endl;
		return ss.str();
	}
};
typedef std::list<TileItem> TileItems;
typedef TileItems::iterator TileItemsIter;
typedef TileItems::reverse_iterator TileItemsRevIter;
typedef TileItems::const_iterator TileItemsConstIter;

class State {
public:
	State (const TileItems& _map, const IMap::SettingsMap& _settingsMap, const std::string& _mapName, int _mapWidth,
			int _mapHeight) :
			map(_map), settingsMap(_settingsMap), mapName(_mapName), mapWidth(_mapWidth), mapHeight(_mapHeight)
	{
	}
	TileItems map;
	IMap::SettingsMap settingsMap;
	std::string mapName;
	int mapWidth;
	int mapHeight;
};

class IMapEditorListener {
public:
	virtual ~IMapEditorListener ()
	{
	}
	virtual void onSettingsValueChange (const std::string& key, const std::string& value)
	{
	}
	virtual void onFileNameChange (const std::string& oldName, const std::string& newName)
	{
	}
	virtual void onMapNameChange (const std::string& oldName, const std::string& newName)
	{
	}
	virtual void onCaveAmountChange (int amount)
	{
	}
	virtual void onTileSelected (const SpriteDefPtr& def)
	{
	}
	virtual void onNewMap ()
	{
	}
	virtual void onTilePlaced (const SpriteDefPtr& def)
	{
	}
	virtual void onTileRemoved (const SpriteDefPtr& def)
	{
	}
};

class UINodeMapEditor: public UINode {
	friend class StateChecker;
private:
	SpriteDefPtr _activeSpriteDefition;
	EntityAngle _activeSpriteAngle;
	const EntityType* _activeEntityType;
	bool _activeEntityTypeRight;
	gridCoord _selectedGridX;
	gridCoord _selectedGridY;
	int _gridScrollX;
	int _gridScrollY;
	float _scale;
	int _mapWidth;
	int _mapHeight;
	MapEditorLayer _activeLayer;
	std::string _fileName;
	std::string _mapName;
	unsigned char _buttonPressed;

	TileItems _map;
	IMap::SettingsMap _settings;

	mutable std::vector<State> _undoStates;
	mutable std::vector<State> _redoStates;

	// the height in grid tiles from the bottom of the map
	float _waterHeight;

	gridCoord _playerX;
	gridCoord _playerY;

	bool _renderGrid;

	std::vector<IMapEditorListener*> _editorListeners;

	IMapManager& _mapManager;
	const ThemeType* _theme;

	TileItem* _highlightItem;

	BitmapFontPtr _font;
	Color _fontColor;

	int _tileWidth;

	ConfigVarPtr _lastMap;

	bool _moveTileHorizontally;

	int _scrollX;
	int _scrollY;
	int32_t _lastScrollUpdate;
	uint32_t _nextScrollDelta;

	// active layers
	int _layerMask;

	int _lastSave;

	// get the grid index on the x-coord - starting from 0
	gridCoord getGridX () const;
	// get the grid index on the y-coord - starting from 0
	gridCoord getGridY () const;

	int getTileWidth () const;
	int getTileHeight () const;
	// get the visible amount of tiles on the screen for the current scale level
	int getScreenMapGridWidth() const;
	// get the visible amount of tiles on the screen for the current scale level
	int getScreenMapGridHeight() const;
	// get the theoretically max visible tiles at the current scale level. This can be higher than the map width
	int getScreenMapGridMaxWidth () const;
	// get the theoretically max visible tiles at the current scale level. This can be higher than the map height
	int getScreenMapGridMaxHeight () const;

	TileItem* getSelectedTile ();

	MapEditorLayer getLayer (const SpriteType& type) const;

	void renderSprite (const TileItem& item, const int x, const int y, float alpha = 1.0f) const;
	void renderGrid (int x, int y) const;
	void renderPlayer (int x, int y) const;
	void renderScrollbars(int x, int y) const;
	void renderHighlightItem (int x, int y) const;
	void renderWater (int x, int y) const;
	void renderBorder (const TileItem& item, int x, int y, const Color& color, bool fullSize, bool inner) const;
	void selectPlacedTile ();
	bool placeTileItem (bool overwrite);
	bool placeTileItem (const SpriteDefPtr& def, const EntityType* entityType, gridCoord gridX, gridCoord gridY,
			MapEditorLayer layer, bool overwrite, EntityAngle angle);
	bool placeEmitter (const SpriteDefPtr& def, const EntityType* entityType, gridCoord gridX, gridCoord gridY,
			int emitterAmount, int emitterDelay, bool overwrite, EntityAngle angle, const std::string& settings);
	bool placeCave (const SpriteDefPtr& def, const EntityType* entityType, gridCoord gridX, gridCoord gridY,
			MapEditorLayer layer, int delay, bool overwrite);
	bool placeTileItem (const TileItem& item, bool overwrite);
	// delete the current selected item
	void deleteItem ();
	// returns true if a tile was hit - or false if the tile is going to get replaced with the given sprite definition
	bool checkTileHit (const TileItem& tileItem, bool remove);
	bool isOverlapping (gridCoord gridX, gridCoord gridY, const TileItem& item) const;
	bool isOverlapping (gridCoord gridX, gridCoord gridY, gridSize width, gridSize height, const TileItem& item) const;
	// check whether item1 hits item2
	bool isOverlapping (const TileItem& item1, const TileItem& item2) const;
	void loadFromContext (IMapContext& ctx);
	void setMapDimensions (int mapWidth, int mapHeight);
	void setFlyingNpc (bool flyingNpc);
	void setFishNpc (bool fishNpc);
	void setPackageTransferCount (int amount);
	void setWaterParameters (float waterHeight, float waterChangeSpeed, uint32_t waterRisingDelay, uint32_t waterFallingDelay);
	void setGravity (float gravity);

	int countCaves () const;
	void notifyCaveAmountChange ();
	void notifyNewMap ();
	void notifyTilePlaced (const SpriteDefPtr& def);
	void notifyTileRemoved (const SpriteDefPtr& def);
	void notifySelectionChange (const SpriteDefPtr& def);

	void setState (const State& state);

	void updateScrolling ();

	void doClear ();
	void shift (int shiftX, int shiftY);
	void moveTile (bool right);
	void scale (float scale);
	// find the optimal scale level to show the whole map
	void fitView ();

	// command callback
	void loadMap (const ICommand::Args& args);

	bool isLayerActive (int layer) const;
public:
	UINodeMapEditor (IFrontend *frontend, IMapManager& mapManager);
	virtual ~UINodeMapEditor ();

	void setSprite (const SpriteDefPtr& spriteDef);
	void setEmitterEntity (const EntityType& type);
	void setFileName (const std::string& fileName);
	void setMapName (const std::string& mapName);
	const std::string& getName () const;
	int getMapHeight () const;

	bool isDirty () const;
	void save ();
	void clear ();
	void undo ();
	void redo ();
	void load (const std::string& mapName);
	void loadLast ();
	void autoFill (const ThemeType& theme);

	void toggleLayer (MapEditorLayer layer);

	void addEditorListener (IMapEditorListener *listener);
	void setSetting (const std::string& key, const std::string& value);

	void setWaterHeight (float waterHeight);
	void setPlayerPosition (gridCoord gridX, gridCoord gridY);

	void setTheme (const ThemeType& theme);

	void toggleGrid ();

	// UINode
	bool isActive () const override;
	void render (int x, int y) const override;
	void removeFocus () override;
	void update (uint32_t deltaTime) override;
	bool onKeyPress (int32_t key, int16_t modifier) override;
	bool onKeyRelease (int32_t key) override;
	bool onMouseWheel (int32_t x, int32_t y) override;
	void onMouseMotion (int32_t x, int32_t y, int32_t relX, int32_t relY) override;
	bool onMouseButtonRelease (int32_t x, int32_t y, unsigned char button) override;
	bool onMouseButtonPress (int32_t x, int32_t y, unsigned char button) override;
};

inline void UINodeMapEditor::setGravity (float gravity)
{
	setSetting(msn::GRAVITY, string::toString(gravity));
}

inline void UINodeMapEditor::setWaterHeight (float waterHeight)
{
	if (_waterHeight > _mapHeight) {
		_waterHeight = string::toFloat(msd::WATER_HEIGHT);
	} else {
		_waterHeight = waterHeight;
	}
	setSetting(msn::WATER_HEIGHT, string::toString(waterHeight));
}

inline void UINodeMapEditor::setPlayerPosition (gridCoord gridX, gridCoord gridY)
{
	_playerX = gridX;
	_playerY = gridY;
	setSetting(msn::PLAYER_X, string::toString(gridX));
	setSetting(msn::PLAYER_Y, string::toString(gridY));
}

inline void UINodeMapEditor::setWaterParameters (float waterHeight, float waterChangeSpeed,
		uint32_t waterRisingDelay, uint32_t waterFallingDelay)
{
	setWaterHeight(waterHeight);
	setSetting(msn::WATER_CHANGE, string::toString(waterChangeSpeed));
	setSetting(msn::WATER_RISING_DELAY, string::toString(waterRisingDelay));
	setSetting(msn::WATER_FALLING_DELAY, string::toString(waterFallingDelay));
}

inline void UINodeMapEditor::setFlyingNpc (bool flyingNpc)
{
	setSetting(msn::FLYING_NPC, flyingNpc ? "true" : "false");
}

inline void UINodeMapEditor::setFishNpc (bool fishNpc)
{
	setSetting(msn::FISH_NPC, fishNpc ? "true" : "false");
}

inline void UINodeMapEditor::setPackageTransferCount (int amount)
{
	setSetting(msn::PACKAGE_TRANSFER_COUNT, string::toString(amount));
}

inline void UINodeMapEditor::setMapDimensions (int mapWidth, int mapHeight)
{
	_mapWidth = mapWidth;
	_mapHeight = mapHeight;
	if (_mapWidth < MIN_WIDTH)
		_mapWidth = MIN_WIDTH;
	if (_mapHeight < MIN_HEIGHT)
		_mapHeight = MIN_HEIGHT;
	setSetting(msn::WIDTH, string::toString(mapWidth));
	setSetting(msn::HEIGHT, string::toString(mapHeight));
}

inline MapEditorLayer UINodeMapEditor::getLayer (const SpriteType& type) const
{
	if (SpriteTypes::isRock(type) || SpriteTypes::isAnyGround(type) || SpriteTypes::isWaterFall(type)) {
		return LAYER_SOLID;
	} else if (SpriteTypes::isBackground(type) || SpriteTypes::isWindow(type) || SpriteTypes::isCave(type)) {
		return LAYER_BACKGROUND;
	} else if (SpriteTypes::isBridge(type)) {
		return LAYER_FOREGROUND;
	} else if (SpriteTypes::isLiane(type)) {
		return LAYER_DECORATION;
	}
	return LAYER_SOLID;
}

inline void UINodeMapEditor::setEmitterEntity (const EntityType& type)
{
	const Animation& animation = EntityTypes::hasDirection(type) ? Animations::ANIMATION_IDLE_RIGHT : Animations::ANIMATION_IDLE;
	const SpriteDefPtr& spriteDef = SpriteDefinition::get().getFromEntityType(type, animation);
	if (!spriteDef)
		return;

	_activeSpriteDefition = spriteDef;
	_activeEntityType = &type;
	_activeSpriteAngle = spriteDef->angle;
	_activeEntityTypeRight = true;
	_activeLayer = LAYER_EMITTER;
}

inline void UINodeMapEditor::setSprite (const SpriteDefPtr& spriteDef)
{
	_activeSpriteDefition = spriteDef;
	_activeEntityType = 0;
	_activeSpriteAngle = spriteDef->angle;
	_activeEntityTypeRight = true;
	_activeLayer = getLayer(_activeSpriteDefition->type);
}

inline void UINodeMapEditor::setMapName (const std::string& mapName)
{
	const std::string old = _mapName;
	_mapName = mapName;
	for (std::vector<IMapEditorListener*>::iterator i = _editorListeners.begin(); i != _editorListeners.end(); ++i) {
		(*i)->onMapNameChange(old, _mapName);
	}
}

inline const std::string& UINodeMapEditor::getName () const
{
	return _fileName;
}

inline int UINodeMapEditor::getMapHeight () const
{
	return _mapHeight;
}

inline void UINodeMapEditor::setTheme (const ThemeType& theme)
{
	_theme = &theme;
	setSetting(msn::THEME, theme.name);
}
