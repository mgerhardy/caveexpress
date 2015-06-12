#pragma once

namespace caveexpress {

class RedoListener: public UINodeListener {
private:
	UINodeMapEditor *_mapEditor;
public:
	explicit RedoListener (UINodeMapEditor *mapEditor) :
			_mapEditor(mapEditor)
	{
	}

	void onClick () override
	{
		_mapEditor->redo();
	}
};

}
