#pragma once

#include "ui/editor/UIMapEditorWindow.h"
#include "caveexpress/client/editor/MapEditorDocument.h"

namespace caveexpress {

class UICaveExpressMapEditorWindow: public UIMapEditorWindow {
protected:
	mutable bool _waterDragging = false;
	mutable bool _showAllTriggerLinks = true;
	mutable bool _layoutChecked = false;
	mutable MapMetrics _layoutMetrics;

	void drawPropertiesPanel () const override;
	void drawScriptExtras () const override;
	bool handleCanvasOverlayInput (float tileW, float tileH) const override;
	void renderCanvasOverlay (ImDrawList* drawList, float originX, float originY, float tileW, float tileH) const override;
	const Animation& getPlayerAnimation () const override;

public:
	UICaveExpressMapEditorWindow (IFrontend* frontend, IMapManager& mapManager);
	MapEditorDocument& ceDocument () { return static_cast<MapEditorDocument&>(document()); }
	MapEditorDocument& ceDocument () const { return static_cast<MapEditorDocument&>(*_doc); }
};

}
