#pragma once

class ToggleGridListener: public UINodeListener {
private:
	UINodeMapEditor *_mapEditor;
public:
	explicit ToggleGridListener (UINodeMapEditor *mapEditor) :
			_mapEditor(mapEditor)
	{
	}

	void onClick () override
	{
		_mapEditor->toggleGrid();
	}
};
