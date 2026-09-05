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
	std::string _scriptLogic;
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
		std::string scriptLogic;
		int mapWidth = 0;
		int mapHeight = 0;

		State () = default;
		State (const MapEditorTileItems& tiles, const IMap::SettingsMap& settings,
				const IMap::StartPositions& starts, const std::string& name, int width, int height,
				const std::string& script = "") :
				map(tiles), settingsMap(settings), startPositions(starts), mapName(name), scriptLogic(script),
				mapWidth(width), mapHeight(height)
		{
		}
	};

	enum class Tool {
		Paint,
		Erase,
		Pick,
		Fill
	};

	/** Palette / erase scope: Tiles tab edits tiles, Entities tab edits emitters. */
	enum class EditMode {
		Tiles,
		Entities
	};

protected:
	IMapManager& _mapManager;
	ConfigVarPtr _lastMap;

	MapEditorTileItems _map;
	IMap::SettingsMap _settings;
	IMap::StartPositions _startPositions;
	std::string _fileName;
	std::string _mapName;
	/** Lua kept across saves (onUpdate/onMapLoaded/helpers); not part of undo. */
	std::string _scriptLogic;
	bool _scriptDirty = false;
	bool _preserveInitMap = false;
	bool _scriptUndoPending = false;
	State _scriptUndoBefore;
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
	EditMode _editMode = EditMode::Tiles;

	bool _hasRegion = false;
	int _regionX0 = 0;
	int _regionY0 = 0;
	int _regionX1 = 0;
	int _regionY1 = 0;
	std::vector<MapEditorTileItem> _clipboard;

	std::vector<State> _undoStates;
	std::vector<State> _redoStates;
	size_t _lastSave = 0;
	bool _undoStrokeActive = false;
	State _undoStrokeSnapshot;

	bool placeTileItem (const MapEditorTileItem& item, bool overwrite);
	bool checkTileHit (const MapEditorTileItem& tileItem, bool remove);
	bool isEntityItem (const MapEditorTileItem& item) const;
	bool matchesEditMode (const MapEditorTileItem& item) const;
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
	virtual bool saveToGameData ();
	bool saveAndPlay ();
	bool saveAndPlayFrom (gridCoord gridX, gridCoord gridY);
	bool isDirty () const;
	void collectValidationIssues (std::vector<std::string>& out) const;
	virtual void collectGameValidationIssues (std::vector<std::string>& out) const;
	/** Treat current undo state as saved (discard unsaved changes without reloading). */
	void discardChanges ();

	void undo ();
	void redo ();
	bool canUndo () const { return !_undoStates.empty(); }
	bool canRedo () const { return !_redoStates.empty(); }
	/** Snapshot once for a paint/erase drag; commit with endUndoStroke(). */
	void beginUndoStroke ();
	void endUndoStroke ();

	void setSprite (const SpriteDefPtr& spriteDef);
	void setEmitterEntity (const EntityType& type);
	SpriteDefPtr findEntitySprite (const EntityType& type) const;
	virtual void setActiveEntityRight (bool right);
	virtual void rotateBrush ();
	/** Rotate the highlighted tile in place, or the brush if nothing rotatable is selected. */
	virtual void rotateSelectionOrBrush ();
	void setTool (Tool tool) { _tool = tool; }
	Tool getTool () const { return _tool; }
	void setEditMode (EditMode mode);
	EditMode getEditMode () const { return _editMode; }

	void setSelectedGrid (gridCoord x, gridCoord y);
	void focusCell (gridCoord x, gridCoord y);
	gridCoord getSelectedGridX () const { return _selectedGridX; }
	gridCoord getSelectedGridY () const { return _selectedGridY; }

	bool paintAtSelection (bool overwrite = true, bool recordUndo = true);
	bool eraseAtSelection (bool recordUndo = true);
	void pickAtSelection ();
	void pickTopmostAtSelection ();
	void deleteSelection ();
	void resizeMap (int mapWidth, int mapHeight);
	void floodFillAtSelection ();
	void setRegion (int x0, int y0, int x1, int y1);
	void clearRegion () { _hasRegion = false; }
	bool hasRegion () const { return _hasRegion; }
	void getRegion (int& x0, int& y0, int& x1, int& y1) const;
	void copyRegion ();
	void pasteAtSelection ();
	void nudgeSelection (int dx, int dy);
	bool hasClipboard () const { return !_clipboard.empty(); }

	MapEditorTileItem* getSelectedTile ();
	MapEditorTileItem* getHighlightItem () { return _highlightItem; }
	const MapEditorTileItem* getHighlightItem () const { return _highlightItem; }
	void setHighlightFromSelection ();

	void toggleLayer (MapEditorLayer layer);
	bool isLayerActive (int layer) const;
	void toggleGrid () { _renderGrid = !_renderGrid; }
	bool isRenderGrid () const { return _renderGrid; }

	void setSetting (const std::string& key, const std::string& value);
	void removeSetting (const std::string& key);
	std::string getSetting (const std::string& key, const std::string& fallback = "") const;
	void setFileName (const std::string& fileName);
	void setMapName (const std::string& mapName);
	void setMapDimensions (int mapWidth, int mapHeight);
	void setTheme (const ThemeType& theme);
	void shift (int shiftX, int shiftY);
	void setPlayerPosition (gridCoord gridX, gridCoord gridY);
	void removeStartPosition (size_t index);
	void setStartPositionAt (size_t index, gridCoord gridX, gridCoord gridY);
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
	/** True when maps are Lua files and script logic can be edited in the editor. */
	virtual bool supportsMapScript () const { return false; }
	virtual const EntityType& getPlayerEntityType () const = 0;
	virtual void changeMapTheme (const ThemeType& toTheme) { setTheme(toTheme); }
	virtual void autoFill (const ThemeType& theme) {}
	virtual void setWaterHeight (float) {}

	const std::string& getScriptLogic () const { return _scriptLogic; }
	std::string& getScriptLogicMutable () { return _scriptLogic; }
	void beginScriptUndo ();
	void markScriptChanged ();
	void endScriptUndo ();
	void setScriptLogic (const std::string& logic);
	void setPreserveInitMap (bool preserve) { _preserveInitMap = preserve; }
	bool isPreserveInitMap () const { return _preserveInitMap; }
	std::string getUserMapsPath () const;
	std::string getGameDataMapsPath () const;
};
