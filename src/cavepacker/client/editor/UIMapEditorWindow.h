#pragma once

#include "ui/editor/UIMapEditorWindow.h"
#include "cavepacker/client/editor/MapEditorDocument.h"

namespace cavepacker {

class UIMapEditorWindow: public ::UIMapEditorWindow {
protected:
	mutable bool _layoutChecked = false;
	mutable int _reachablePlayable = 0;
	mutable int _playableCells = 0;
	mutable std::string _layoutFailure;

	void drawPropertiesPanel () const override;
	void drawHelpDocs () const override;
	void drawHelpExtras () const override;
	std::string getPlayFromHereTooltip () const override;

public:
	UIMapEditorWindow (IFrontend* frontend, IMapManager& mapManager);
	MapEditorDocument& sokobanDocument () { return static_cast<MapEditorDocument&>(document()); }
	MapEditorDocument& sokobanDocument () const { return static_cast<MapEditorDocument&>(*_doc); }
};

}
