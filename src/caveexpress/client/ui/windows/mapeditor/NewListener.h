#pragma once

class NewListener: public UINodeListener {
private:
	UINodeMapEditor *_mapEditor;
public:
	NewListener (UINodeMapEditor *mapEditor) :
			_mapEditor(mapEditor)
	{
	}

	void onClick () override
	{
		_mapEditor->clear();
	}
};
