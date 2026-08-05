#pragma once

#include "ui/windows/UIWindow.h"
#include "ui/editor/IMapEditorDocument.h"
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
	mutable bool _showScriptEditor = false;
	mutable std::string _confirmAction;
	mutable char _tileFilter[128] = {};
	mutable char _entityFilter[128] = {};
	mutable char _mapFilter[128] = {};
	mutable char _fileNameBuf[128] = {};
	mutable char _mapTitleBuf[256] = {};
	mutable std::vector<SpriteDefPtr> _tilePalette;
	mutable std::vector<const EntityType*> _entityPalette;
	mutable const ThemeType* _paletteTheme = nullptr;

	void rebuildPalettes () const;
	void fitView () const;
	void handleHotkeys () const;
	void drawToolbar () const;
	void drawTilesPanel () const;
	void drawEntitiesPanel () const;
	void drawLayersPanel () const;
	void drawMapsPanel () const;
	void drawHelpPanel () const;
	void drawConfirmModal () const;
	void drawScriptEditor () const;
	void drawCanvas () const;
	void renderMapIntoCanvas (ImDrawList* drawList) const;
	void renderSprite (ImDrawList* drawList, const MapEditorTileItem& item, float originX, float originY,
			float tileW, float tileH, float alpha = 1.0f) const;
	bool requestAction (const char* action) const;
	void executePendingAction () const;
	void leaveEditor () const;
	float tileWidth () const;
	float tileHeight () const;

	virtual void drawPropertiesPanel () const;
	virtual void drawHelpExtras () const {}
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
