#pragma once

#include "ui/windows/UIWindow.h"
#include "ui/editor/IMapEditorDocument.h"
#include "ui/editor/ImGuiTextureDraw.h"
#include "ui/editor/UIShapeEditor.h"
#include "common/Animation.h"
#include "imgui.h"
#include <memory>
#include <string>
#include <vector>

class UIMapEditorWindow: public UIWindow {
protected:
	std::unique_ptr<IMapEditorDocument> _doc;
	IFrontend* _frontendPtr;

	mutable float _panX = 0.0f;
	mutable float _panY = 0.0f;
	mutable float _zoom = 1.0f;
	mutable int _tileRefWidth = 16;
	mutable float _canvasMinX = 0.0f;
	mutable float _canvasMinY = 0.0f;
	mutable float _canvasMaxX = 0.0f;
	mutable float _canvasMaxY = 0.0f;
	mutable bool _canvasHovered = false;
	mutable bool _panning = false;
	mutable bool _showHelp = false;
	mutable bool _showConfirm = false;
	mutable bool _focusScriptPanel = false;
	mutable bool _showValidation = false;
	mutable bool _validationSaveGameData = false;
	mutable bool _regionDragging = false;
	mutable bool _movingItem = false;
	mutable bool _moveUndoStarted = false;
	mutable gridCoord _moveGrabOffsetX = 0.0f;
	mutable gridCoord _moveGrabOffsetY = 0.0f;
	mutable bool _previewAltAtlas = false;
	mutable int _nativeTileRefWidth = 16;
	mutable UIShapeEditor _shapeEditor;
	mutable std::string _confirmAction;
	mutable std::vector<std::string> _validationIssues;
	mutable char _tileFilter[128] = {};
	mutable char _entityFilter[128] = {};
	mutable char _mapFilter[128] = {};
	mutable char _fileNameBuf[128] = {};
	mutable char _mapTitleBuf[256] = {};
	mutable char _scriptFind[128] = {};
	mutable char _scriptReplace[128] = {};
	mutable int _regionAnchorX = 0;
	mutable int _regionAnchorY = 0;
	mutable IMapEditorDocument::MapEdge _mapEdgeHover = IMapEditorDocument::MapEdge::None;
	mutable IMapEditorDocument::MapEdge _mapEdgeDragging = IMapEditorDocument::MapEdge::None;
	mutable float _mapResizeStartMouseX = 0.0f;
	mutable float _mapResizeStartMouseY = 0.0f;
	mutable int _mapResizeApplied = 0;
	mutable std::vector<SpriteDefPtr> _tilePalette;
	mutable std::vector<const EntityType*> _entityPalette;
	mutable const ThemeType* _paletteTheme = nullptr;
	mutable bool _showDefinition = false;
	mutable bool _definitionIsEntity = false;
	mutable std::string _definitionId;
	mutable std::string _definitionLua;
	mutable std::string _definitionStatus;
	mutable SpriteDefPtr _definitionSprite;
	mutable const EntityType* _definitionEntity = nullptr;
	mutable std::vector<MapEditorSpriteQuad> _mapSpriteBatch;

	void rebuildPalettes () const;
	void fitView () const;
	void centerViewOnGrid (gridCoord x, gridCoord y) const;
	void handleHotkeys () const;
	void drawToolbar () const;
	void drawTilesPanel () const;
	void drawEntitiesPanel () const;
	void drawTileContextMenu (const SpriteDefPtr& sprite) const;
	void drawEntityContextMenu (const EntityType& type) const;
	void openSpriteDefinition (const SpriteDefPtr& sprite) const;
	void openEntityDefinition (const EntityType& type) const;
	void drawDefinitionEditor () const;
	bool saveDefinitionEditor () const;
	void drawLayersPanel () const;
	void drawMapsPanel () const;
	void drawHelpPanel () const;
	void drawConfirmModal () const;
	void drawValidationModal () const;
	void drawScriptEditor () const;
	bool trySave (bool toGameData = false) const;
	void applyScriptFind (bool replaceAll) const;
	void drawCanvas () const;
	void setupEditorDockSpace () const;
	void renderMapIntoCanvas (ImDrawList* drawList) const;
	IMapEditorDocument::MapEdge hitTestMapEdge (float tileW, float tileH) const;
	bool handleMapEdgeResize (float tileW, float tileH, bool allowHover) const;
	void renderMapResizeHandles (ImDrawList* drawList, float originX, float originY, float tileW, float tileH) const;
	void collectSprite (std::vector<MapEditorSpriteQuad>& quads, const MapEditorTileItem& item,
			float originX, float originY, float tileW, float tileH, float alpha = 1.0f) const;
	void renderSprite (ImDrawList* drawList, const MapEditorTileItem& item, float originX, float originY,
			float tileW, float tileH, float alpha = 1.0f) const;
	void renderItemBounds (ImDrawList* drawList, const MapEditorTileItem& item, float originX, float originY,
			float tileW, float tileH, ImU32 lineCol, ImU32 fillCol, float thickness) const;
	bool requestAction (const char* action) const;
	void executePendingAction () const;
	void leaveEditor () const;
	float tileWidth () const;
	float tileHeight () const;

	virtual void drawPropertiesPanel () const;
	bool beginPropertiesGroup (const char* label, bool defaultOpen = true) const;
	virtual void drawHelpDocs () const;
	virtual void drawHelpExtras () const {}
	virtual void drawScriptExtras () const {}
	virtual std::string getPlayFromHereTooltip () const;
	virtual bool handleCanvasOverlayInput (float /*tileW*/, float /*tileH*/) const { return false; }
	virtual void renderCanvasOverlay (ImDrawList* drawList, float originX, float originY, float tileW, float tileH) const {}
	virtual const Animation& getPlayerAnimation () const { return Animation::NONE; }

public:
	UIMapEditorWindow (IFrontend* frontend, std::unique_ptr<IMapEditorDocument> doc);
	virtual ~UIMapEditorWindow ();

	IMapEditorDocument& document () { return *_doc; }

	void render (int x, int y) const override;
	bool onPop () override;
	bool onPush () override;
};
