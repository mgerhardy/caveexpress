#pragma once

class ToggleGridListener: public UINodeListener {
private:
	UINodeMapEditor *_mapEditor;
public:
	ToggleGridListener (UINodeMapEditor *mapEditor) :
			_mapEditor(mapEditor)
	{
	}

	void onClick () override
	{
		_mapEditor->toggleGrid();
	}
};
