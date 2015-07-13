#pragma once

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
