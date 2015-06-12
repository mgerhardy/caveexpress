#pragma once

namespace caveexpress {

class NewListener: public UINodeListener {
private:
	UINodeMapEditor *_mapEditor;
public:
	explicit NewListener (UINodeMapEditor *mapEditor) :
			_mapEditor(mapEditor)
	{
	}

	void onClick () override
	{
		_mapEditor->clear();
	}
};

}
