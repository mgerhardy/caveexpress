#pragma once

class UndoListener: public UINodeListener {
private:
	UINodeMapEditor *_mapEditor;
public:
	UndoListener (UINodeMapEditor *mapEditor) :
			_mapEditor(mapEditor)
	{
	}

	void onClick () override
	{
		_mapEditor->undo();
	}
};
