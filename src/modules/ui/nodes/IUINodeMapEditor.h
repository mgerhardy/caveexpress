#pragma once

#include "ui/nodes/UINode.h"
#include "sprites/Sprite.h"
#include "ui/BitmapFont.h"
#include "common/MapManager.h"
#include "common/SpriteDefinition.h"
#include "common/MapSettings.h"
#include "common/EntityType.h"
#include "common/SpriteType.h"
#include "common/Animation.h"
#include "common/String.h"
#include "common/ConfigVar.h"
#include "common/File.h"
#include "common/ICommand.h"
#include "common/IMap.h"
#include <vector>
#include <list>
#include <map>

// forward decl
class IMapManager;
class IMapContext;
class StateChecker;
class IUINodeMapEditor;

namespace {
const int MIN_WIDTH = 6;
const int MIN_HEIGHT = 4;
}

#define Undo() StateChecker s(this, _map, _settings, _startPositions, _mapName, _mapWidth, _mapHeight)

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
	const IUINodeMapEditor *_editor;
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

	bool operator== (const TileItem &other) const;

	gridCoord getX (bool useShape = false) const;

	gridCoord getY (bool useShape = false) const;

	vec2 getSize (bool useShape = false) const;

	bool operator< (const TileItem &other) const;
};
typedef std::list<TileItem> TileItems;
typedef TileItems::iterator TileItemsIter;
typedef TileItems::reverse_iterator TileItemsRevIter;
typedef TileItems::const_iterator TileItemsConstIter;

class StateChecker {
private:
	IUINodeMapEditor *_editor;
	TileItems _map;
	IMap::SettingsMap _settings;
	IMap::StartPositions _startPositions;
	std::string _mapName;
	int _mapWidth;
	int _mapHeight;

public:
	StateChecker(IUINodeMapEditor *editor, const TileItems& map, const IMap::SettingsMap& settings,
			const IMap::StartPositions& startPositions, const std::string& mapName, int mapWidth, int mapHeight);
	~StateChecker ();

	bool checkAndStoreDirtyState (const std::string& id, bool dirty);
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

class IUINodeMapEditor: public UINode {
	friend class StateChecker;
protected:
	static bool layerSort (const TileItem& tileItem1, const TileItem& tileItem2)
	{
		return tileItem1.layer <= tileItem2.layer;
	}

	struct State {
		State (const TileItems& _tileitems, const IMap::SettingsMap& _settingsMap, const IMap::StartPositions& _startPos, const std::string& _mapname, int _mapwidth,
				int _mapheight) :
				map(_tileitems), settingsMap(_settingsMap), startPositions(_startPos), mapName(_mapname), mapWidth(_mapwidth), mapHeight(_mapheight)
		{
		}
		TileItems map;
		IMap::SettingsMap settingsMap;
		IMap::StartPositions startPositions;
		std::string mapName;
		int mapWidth;
		int mapHeight;
	};

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
	IMap::StartPositions _startPositions;

	mutable std::vector<State> _undoStates;
	mutable std::vector<State> _redoStates;

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

	size_t _lastSave;

	virtual void renderPlayer (int x, int y) const = 0;
	virtual IMapContext* getContext (const std::string& mapname) = 0;
	virtual const Animation& getEmitterAnimation (const EntityType& type) const = 0;
public:
	virtual bool isMapTile (const SpriteType &type) const = 0;
	virtual bool isPlayer (const EntityType &type) const = 0;

protected:
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

	virtual MapEditorLayer getLayer (const SpriteType& type) const {
		return LAYER_SOLID;
	}

	void renderSprite (const TileItem& item, const int x, const int y, float alpha = 1.0f) const;
	void renderGrid (int x, int y) const;
	void renderScrollbars(int x, int y) const;
	virtual void renderHighlightItem (int x, int y) const;
	void renderBorder (const TileItem& item, int x, int y, const Color& color, bool fullSize, bool inner) const;
	void selectPlacedTile ();
	bool placeTileItem (bool overwrite);
	virtual bool placeTileItem (const SpriteDefPtr& def, const EntityType* entityType, gridCoord gridX, gridCoord gridY,
			MapEditorLayer layer, bool overwrite, EntityAngle angle);
	virtual bool placeEmitter (const SpriteDefPtr& def, const EntityType* entityType, gridCoord gridX, gridCoord gridY,
			int emitterAmount, int emitterDelay, bool overwrite, EntityAngle angle, const std::string& settings);
	bool placeTileItem (const TileItem& item, bool overwrite);
	// delete the current selected item
	virtual void deleteItem ();
	// returns true if a tile was hit - or false if the tile is going to get replaced with the given sprite definition
	bool checkTileHit (const TileItem& tileItem, bool remove);
	bool isOverlapping (gridCoord gridX, gridCoord gridY, const TileItem& item) const;
	bool isOverlapping (gridCoord gridX, gridCoord gridY, gridSize width, gridSize height, const TileItem& item) const;
	// check whether item1 hits item2
	virtual bool isOverlapping (const TileItem& item1, const TileItem& item2) const;
	virtual void loadFromContext (IMapContext& ctx);
	void setMapDimensions (int mapWidth, int mapHeight);

	void notifyNewMap ();
	void notifyTilePlaced (const SpriteDefPtr& def);
	void notifyTileRemoved (const SpriteDefPtr& def);
	void notifySelectionChange (const SpriteDefPtr& def);
	virtual void setState (const State& state);

	void updateScrolling ();

	virtual bool shouldSaveTile (const TileItem& tile) const;
	virtual bool shouldSaveEmitter (const TileItem& tile) const;

	virtual void doClear ();
	void shift (int shiftX, int shiftY);
	void moveTile (bool right);
	void scale (float scale);
	// find the optimal scale level to show the whole map
	void fitView ();

	// command callback
	void loadMap (const ICommand::Args& args);
	virtual void initNewMap();
	virtual void onRotate();

	bool isLayerActive (int layer) const;
public:
	IUINodeMapEditor (IFrontend *frontend, IMapManager& mapManager);
	virtual ~IUINodeMapEditor ();

	void setSprite (const SpriteDefPtr& spriteDef);
	void setEmitterEntity (const EntityType& type);
	void setFileName (const std::string& fileName);
	void setMapName (const std::string& mapName);
	const std::string& getName () const;
	int getMapHeight () const;

	bool isDirty () const;
	virtual bool save ();
	virtual void prepareContextForSaving(IMapContext* ctx);
	void clear ();
	void undo ();
	void redo ();
	virtual void load (const std::string& mapName);
	void loadLast ();

	void toggleLayer (MapEditorLayer layer);

	void addEditorListener (IMapEditorListener *listener);
	void setSetting (const std::string& key, const std::string& value);
	void setTheme (const ThemeType& theme);
	void setPlayerPosition (gridCoord gridX, gridCoord gridY);

	void toggleGrid ();

	// UINode
	virtual bool isActive () const override;
	virtual void render (int x, int y) const override;
	virtual void removeFocus (UIFocusRemovalReason reason) override;
	virtual void update (uint32_t deltaTime) override;
	virtual bool onKeyPress (int32_t key, int16_t modifier) override;
	virtual bool onKeyRelease (int32_t key) override;
	virtual bool onMouseWheel (int32_t x, int32_t y) override;
	virtual void onMouseMotion (int32_t x, int32_t y, int32_t relX, int32_t relY) override;
	virtual bool onMouseButtonRelease (int32_t x, int32_t y, unsigned char button) override;
	virtual bool onMouseButtonPress (int32_t x, int32_t y, unsigned char button) override;
};

inline void IUINodeMapEditor::setPlayerPosition (gridCoord gridX, gridCoord gridY)
{
	const IMap::StartPosition p{ string::toString(gridX), string::toString(gridY) };
	for (const IMap::StartPosition& position : _startPositions) {
		if (position._x == p._x && position._y == p._y)
			return;
	}
	_startPositions.push_back(p);
}

inline void IUINodeMapEditor::setMapDimensions (int mapWidth, int mapHeight)
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

inline void IUINodeMapEditor::setEmitterEntity (const EntityType& type)
{
	const SpriteDefPtr& spriteDef = SpriteDefinition::get().getFromEntityType(type, getEmitterAnimation(type));
	if (!spriteDef)
		return;

	_activeSpriteDefition = spriteDef;
	_activeEntityType = &type;
	_activeSpriteAngle = spriteDef->angle;
	_activeEntityTypeRight = true;
	_activeLayer = LAYER_EMITTER;
}

inline void IUINodeMapEditor::setSprite (const SpriteDefPtr& spriteDef)
{
	_activeSpriteDefition = spriteDef;
	_activeEntityType = nullptr;
	_activeSpriteAngle = spriteDef->angle;
	_activeEntityTypeRight = true;
	_activeLayer = getLayer(_activeSpriteDefition->type);
}

inline void IUINodeMapEditor::setMapName (const std::string& mapName)
{
	const std::string old = _mapName;
	_mapName = mapName;
	for (std::vector<IMapEditorListener*>::iterator i = _editorListeners.begin(); i != _editorListeners.end(); ++i) {
		(*i)->onMapNameChange(old, _mapName);
	}
}

inline const std::string& IUINodeMapEditor::getName () const
{
	return _fileName;
}

inline int IUINodeMapEditor::getMapHeight () const
{
	return _mapHeight;
}

inline void IUINodeMapEditor::setTheme (const ThemeType& theme)
{
	_theme = &theme;
	setSetting(msn::THEME, theme.name);
}
