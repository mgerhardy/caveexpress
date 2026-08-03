#pragma once

#include "ui/editor/MapEditorTypes.h"
#include "common/IMap.h"
#include "common/MapManager.h"
#include "common/ThemeType.h"
#include "common/ConfigVar.h"
#include "common/ICommand.h"
#include "common/IMapContext.h"
#include "common/Animation.h"
#include <vector>
#include <string>
#include <memory>

class IMapEditorDocument;

class MapEditorStateChecker {
private:
	IMapEditorDocument* _doc;
	MapEditorTileItems _map;
	IMap::SettingsMap _settings;
	IMap::StartPositions _startPositions;
	std::string _mapName;
	int _mapWidth;
	int _mapHeight;
public:
	explicit MapEditorStateChecker (IMapEditorDocument* doc);
	~MapEditorStateChecker ();
};

#define MapEditorUndo() MapEditorStateChecker _mapEditorUndoGuard(this)

class IMapEditorDocument {
	friend class MapEditorStateChecker;
public:
	static const int MIN_WIDTH;
	static const int MIN_HEIGHT;

	struct State {
		MapEditorTileItems map;
		IMap::SettingsMap settingsMap;
		IMap::StartPositions startPositions;
		std::string mapName;
		int mapWidth = 0;
		int mapHeight = 0;

		State () = default;
		State (const MapEditorTileItems& tiles, const IMap::SettingsMap& settings,
				const IMap::StartPositions& starts, const std::string& name, int width, int height) :
				map(tiles), settingsMap(settings), startPositions(starts), mapName(name), mapWidth(width), mapHeight(height)
		{
		}
	};

	enum class Tool {
		Paint,
		Erase,
		Pick
	};

protected:
	IMapManager& _mapManager;
	ConfigVarPtr _lastMap;

	MapEditorTileItems _map;
	IMap::SettingsMap _settings;
	IMap::StartPositions _startPositions;
	std::string _fileName;
	std::string _mapName;
	int _mapWidth = 16;
	int _mapHeight = 12;
	const ThemeType* _theme;

	SpriteDefPtr _activeSprite;
	EntityAngle _activeAngle = 0;
	const EntityType* _activeEntityType = nullptr;
	bool _activeEntityRight = true;
	MapEditorLayer _activeLayer = LAYER_FOREGROUND;
	int _emitterAmount = 1;
	int _emitterDelay = 0;

	gridCoord _selectedGridX = 0.0f;
	gridCoord _selectedGridY = 0.0f;
	MapEditorTileItem* _highlightItem = nullptr;

	int _layerMask = 0xFFFFFFFF;
	bool _renderGrid = true;
	Tool _tool = Tool::Paint;

	std::vector<State> _undoStates;
	std::vector<State> _redoStates;
	size_t _lastSave = 0;
	bool _undoStrokeActive = false;
	State _undoStrokeSnapshot;

	bool placeTileItem (const MapEditorTileItem& item, bool overwrite);
	bool checkTileHit (const MapEditorTileItem& tileItem, bool remove);
	virtual bool isOverlapping (const MapEditorTileItem& item1, const MapEditorTileItem& item2) const;
	bool isOverlapping (gridCoord gridX, gridCoord gridY, gridSize width, gridSize height, const MapEditorTileItem& item) const;
	bool isOverlapping (gridCoord gridX, gridCoord gridY, const MapEditorTileItem& item) const;
	void setState (const State& state);
	State captureState () const;
	bool stateDiffersFrom (const State& state) const;

	virtual MapEditorLayer getLayer (const SpriteType& type) const = 0;
	virtual bool isMapTileType (const SpriteType& type) const = 0;
	virtual bool isPlayerType (const EntityType& type) const = 0;
	virtual const Animation& getEmitterAnimation (const EntityType& type) const = 0;
	/**
	 * Game-specific brush placement rules. Returning false rejects the paint
	 * without modifying the map. Not applied when loading existing maps.
	 */
	virtual bool canPlaceTileItem (const MapEditorTileItem& item) const;
	virtual bool shouldSaveTile (const MapEditorTileItem& tile) const;
	virtual bool shouldSaveEmitter (const MapEditorTileItem& tile) const;
	virtual void doClear ();
	virtual void onAfterStateRestored () {}
	virtual bool placeBrushItem (bool overwrite);
	virtual void prepareContextForSaving (IMapContext& ctx);
	virtual void loadFromContext (IMapContext& ctx);
	virtual std::unique_ptr<IMapContext> createContext (const std::string& mapName) const = 0;
	virtual void fillTilePalette (std::vector<SpriteDefPtr>& out) const = 0;
	virtual void fillEntityPalette (std::vector<const EntityType*>& out) const = 0;

public:
	explicit IMapEditorDocument (IMapManager& mapManager);
	virtual ~IMapEditorDocument ();

	void registerCommands ();
	void unregisterCommands ();

	void newMap ();
	bool load (const std::string& mapName);
	void loadLast ();
	virtual bool save ();
	bool saveAndPlay ();
	bool isDirty () const;

	void undo ();
	void redo ();
	bool canUndo () const { return !_undoStates.empty(); }
	bool canRedo () const { return !_redoStates.empty(); }
	/** Snapshot once for a paint/erase drag; commit with endUndoStroke(). */
	void beginUndoStroke ();
	void endUndoStroke ();

	void setSprite (const SpriteDefPtr& spriteDef);
	void setEmitterEntity (const EntityType& type);
	virtual void setActiveEntityRight (bool right);
	virtual void rotateBrush ();
	void setTool (Tool tool) { _tool = tool; }
	Tool getTool () const { return _tool; }

	void setSelectedGrid (gridCoord x, gridCoord y);
	gridCoord getSelectedGridX () const { return _selectedGridX; }
	gridCoord getSelectedGridY () const { return _selectedGridY; }

	bool paintAtSelection (bool overwrite = true, bool recordUndo = true);
	bool eraseAtSelection (bool recordUndo = true);
	void pickAtSelection ();
	void deleteSelection ();
	void resizeMap (int mapWidth, int mapHeight);

	MapEditorTileItem* getSelectedTile ();
	MapEditorTileItem* getHighlightItem () { return _highlightItem; }
	const MapEditorTileItem* getHighlightItem () const { return _highlightItem; }
	void setHighlightFromSelection ();

	void toggleLayer (MapEditorLayer layer);
	bool isLayerActive (int layer) const;
	void toggleGrid () { _renderGrid = !_renderGrid; }
	bool isRenderGrid () const { return _renderGrid; }

	void setSetting (const std::string& key, const std::string& value);
	std::string getSetting (const std::string& key, const std::string& fallback = "") const;
	void setFileName (const std::string& fileName);
	void setMapName (const std::string& mapName);
	void setMapDimensions (int mapWidth, int mapHeight);
	void setTheme (const ThemeType& theme);
	void shift (int shiftX, int shiftY);
	void setPlayerPosition (gridCoord gridX, gridCoord gridY);
	void setEmitterAmount (int amount) { _emitterAmount = amount; }
	void setEmitterDelay (int delay) { _emitterDelay = delay; }
	int getEmitterAmount () const { return _emitterAmount; }
	int getEmitterDelay () const { return _emitterDelay; }

	const std::string& getFileName () const { return _fileName; }
	const std::string& getMapName () const { return _mapName; }
	int getMapWidth () const { return _mapWidth; }
	int getMapHeight () const { return _mapHeight; }
	virtual float getWaterHeight () const { return 0.0f; }
	const ThemeType& getTheme () const { return *_theme; }
	const MapEditorTileItems& getTiles () const { return _map; }
	const IMap::StartPositions& getStartPositions () const { return _startPositions; }
	const IMap::SettingsMap& getSettings () const { return _settings; }
	const SpriteDefPtr& getActiveSprite () const { return _activeSprite; }
	const EntityType* getActiveEntityType () const { return _activeEntityType; }
	EntityAngle getActiveAngle () const { return _activeAngle; }
	bool isActiveEntityRight () const { return _activeEntityRight; }
	MapEditorLayer getActiveLayer () const { return _activeLayer; }
	IMapManager& getMapManager () { return _mapManager; }

	void collectTilePalette (std::vector<SpriteDefPtr>& out) const { fillTilePalette(out); }
	void collectEntityPalette (std::vector<const EntityType*>& out) const { fillEntityPalette(out); }

	virtual bool supportsThemeControls () const { return false; }
	virtual bool supportsWater () const { return false; }
	virtual bool supportsEmitterParams () const { return false; }
	virtual const EntityType& getPlayerEntityType () const = 0;
	virtual void changeMapTheme (const ThemeType& toTheme) { setTheme(toTheme); }
	virtual void autoFill (const ThemeType& theme) {}
	virtual void setWaterHeight (float) {}
};
