#pragma once

#include "ui/editor/UIMapEditorWindow.h"
#include "caveexpress/client/editor/MapEditorDocument.h"

namespace caveexpress {

class UICaveExpressMapEditorWindow: public UIMapEditorWindow {
protected:
	void drawPropertiesPanel () const override;
	void renderCanvasOverlay (ImDrawList* drawList, float originX, float originY, float tileW, float tileH) const override;
	const Animation& getPlayerAnimation () const override;

public:
	UICaveExpressMapEditorWindow (IFrontend* frontend, IMapManager& mapManager);
	MapEditorDocument& ceDocument () { return static_cast<MapEditorDocument&>(document()); }
	MapEditorDocument& ceDocument () const { return static_cast<MapEditorDocument&>(*_doc); }
};

}
