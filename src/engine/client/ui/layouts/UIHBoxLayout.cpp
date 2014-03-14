#include "UIHBoxLayout.h"
#include "engine/client/ui/nodes/UINode.h"

UIHBoxLayout::UIHBoxLayout (float spacing, bool expandChildren) :
		IUILayout(), _spacing(spacing), _expandChildren(expandChildren)
{
}

UIHBoxLayout::~UIHBoxLayout ()
{
}

void UIHBoxLayout::layout (UINode* parent)
{
	float currentSize = 2.0f * parent->getPadding();
	float minHeight = 0.0f;

	for (UINodeListIter i = _nodes.begin(); i != _nodes.end(); ++i) {
		UINode* node = *i;
		currentSize += node->getWidth() + _spacing;
		minHeight = std::max(node->getHeight() + 2.0f * parent->getPadding(), minHeight);
	}

	float size = currentSize - _spacing;
	parent->setSize(size, minHeight);

	// the size might change the alignment and thus the position of the parent
	float currentPos = parent->getPadding();
	for (UINodeListIter i = _nodes.begin(); i != _nodes.end(); ++i) {
		UINode* node = *i;
		node->setPos(currentPos, node->getY());
		if (_expandChildren)
			node->setSize(node->getWidth(), parent->getHeight() - 2.0f * parent->getPadding());
		currentPos += node->getWidth() + _spacing;
	}
}
