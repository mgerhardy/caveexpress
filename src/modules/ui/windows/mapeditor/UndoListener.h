#pragma once

class UndoListener: public UINodeListener {
private:
	IUINodeMapEditor *_mapEditor;
public:
	explicit UndoListener (IUINodeMapEditor *mapEditor) :
			_mapEditor(mapEditor)
	{
	}

	void onClick () override
	{
		_mapEditor->undo();
	}
};
