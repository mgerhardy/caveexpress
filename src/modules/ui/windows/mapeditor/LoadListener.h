#pragma once

class LoadListener: public UINodeListener {
private:
	UINodeButton *_loadNode;
	UINodeMapStringSelector *_mapListNode;
public:
	LoadListener (UINodeButton *loadNode, UINodeMapStringSelector *mapListNode) :
			_loadNode(loadNode), _mapListNode(mapListNode)
	{
	}

	void onClick () override
	{
		_loadNode->setVisible(false);
		_mapListNode->setVisible(true);
		_mapListNode->reset();
	}
};
