#pragma once

class RedoListener: public UINodeListener {
private:
	UINodeMapEditor *_mapEditor;
public:
	RedoListener (UINodeMapEditor *mapEditor) :
			_mapEditor(mapEditor)
	{
	}

	void onClick () override
	{
		_mapEditor->redo();
	}
};
