#pragma once

namespace caveexpress {

class LoadListListener: public UINodeListener {
private:
	UINodeMapEditor *_mapEditor;
	UINodeButton *_loadNode;
	UINodeMapStringSelector *_mapListNode;
public:
	LoadListListener (UINodeMapEditor *mapEditor, UINodeButton *loadNode, UINodeMapStringSelector *mapListNode) :
			_mapEditor(mapEditor), _loadNode(loadNode), _mapListNode(mapListNode)
	{
	}

	void onClick () override
	{
		std::string* mapName = _mapListNode->getSelection();
		if (!mapName)
			return;
		_mapEditor->load(*mapName);
		onRemoveFocus();
	}

	void onRemoveFocus () override
	{
		_loadNode->setVisible(true);
		_mapListNode->setVisible(false);
	}
};

}
