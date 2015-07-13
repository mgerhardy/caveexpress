#pragma once

class RedoListener: public UINodeListener {
private:
	IUINodeMapEditor *_mapEditor;
public:
	explicit RedoListener (IUINodeMapEditor *mapEditor) :
			_mapEditor(mapEditor)
	{
	}

	void onClick () override
	{
		_mapEditor->redo();
	}
};
