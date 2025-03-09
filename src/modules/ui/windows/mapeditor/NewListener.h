#pragma once

#include "ui/nodes/IUINodeMapEditor.h"

class NewListener: public UINodeListener {
private:
	IUINodeMapEditor *_mapEditor;
public:
	explicit NewListener (IUINodeMapEditor *mapEditor) :
			_mapEditor(mapEditor)
	{
	}

	void onClick () override
	{
		_mapEditor->clear();
	}
};
