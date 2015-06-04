#pragma once

class UndoListener: public UINodeListener {
private:
	UINodeMapEditor *_mapEditor;
public:
	explicit UndoListener (UINodeMapEditor *mapEditor) :
			_mapEditor(mapEditor)
	{
	}

	void onClick () override
	{
		_mapEditor->undo();
	}
};
