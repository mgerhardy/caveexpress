#pragma once

class LoadListListener: public UINodeListener {
private:
	IUINodeMapEditor *_mapEditor;
	UINodeButton *_loadNode;
	UINodeMapStringSelector *_mapListNode;
public:
	LoadListListener (IUINodeMapEditor *mapEditor, UINodeButton *loadNode, UINodeMapStringSelector *mapListNode) :
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
