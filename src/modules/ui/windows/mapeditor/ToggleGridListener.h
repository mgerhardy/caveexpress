#pragma once

#include "ui/nodes/IUINodeMapEditor.h"

class ToggleGridListener: public UINodeListener {
private:
	IUINodeMapEditor *_mapEditor;
public:
	explicit ToggleGridListener (IUINodeMapEditor *mapEditor) :
			_mapEditor(mapEditor)
	{
	}

	void onClick () override
	{
		_mapEditor->toggleGrid();
	}
};
