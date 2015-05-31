#pragma once

#include <vector>
#include "common/Compiler.h"

class UINode;

class IUILayout {
protected:
	typedef std::vector<UINode*> UINodeList;
	typedef UINodeList::iterator UINodeListIter;
	typedef UINodeList::const_iterator UINodeListConstIter;
	typedef UINodeList::reverse_iterator UINodeListRevIter;
	typedef UINodeList::const_reverse_iterator UINodeListConstRevIter;
	UINodeList _nodes;

public:
	IUILayout ()
	{
	}

	virtual ~IUILayout ()
	{
		_nodes.clear();
	}

	void addNode (UINode* node);

	virtual void layout (UINode* parent) = 0;
};

inline void IUILayout::addNode (UINode* node)
{
	_nodes.push_back(node);
}
